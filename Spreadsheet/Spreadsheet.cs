using SpreadsheetUtilities;
using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Xml;

using System.IO;


//Author Chenxi Sun uid u0455173



namespace SS
{



    /// <summary>
    /// The Spreadsheet class is a class where you can set cell content to string double or formula
    /// It will throw invalidNameException if the name is null and argumentNullException if the formula
    /// or text are null 
    /// </summary>
    /// 




    public class Spreadsheet : AbstractSpreadsheet
    {




        private Dictionary<string, Cell> spreadsheet;
        private DependencyGraph graph;
        private Boolean changed;

        /// <summary>
        /// constructor for spreadsheet with no parameters
        /// 
        /// </summary>
        public Spreadsheet() : base(s => true, s => s, "default")
        {
            spreadsheet = new Dictionary<string, Cell>();
            graph = new DependencyGraph();
            changed = false;

        }

        /// <summary>
        /// constructor for spreadsheet that allows parameter of validator, normalizer and version
        /// 
        /// </summary>
        /// <param name="isValid"></param>
        /// <param name="Normalizer"></param>
        /// <param name="Version"></param>

        public Spreadsheet(Func<string, bool> isValid, Func<string, string> Normalizer, string Version) :
            base(isValid, Normalizer, Version)
        {
            spreadsheet = new Dictionary<string, Cell>();
            graph = new DependencyGraph();
            changed = false;

        }

        /// <summary>
        /// constructor for spreadsheet that allows parameter of filepath. 
        /// 
        /// </summary>
        /// <param name="FilePath"></param>
        /// <param name="isValid"></param>
        /// <param name="Normalizer"></param>
        /// <param name="Version"></param>

        public Spreadsheet(string filepath, Func<string, bool> isValid, Func<string, string> Normalizer, string Version) :
          base(isValid, Normalizer, Version)
        {
            spreadsheet = new Dictionary<string, Cell>();
            graph = new DependencyGraph();

            if (!(GetSavedVersion(filepath).Equals(Version)))
            {
                throw new SpreadsheetReadWriteException("There is no file with the version number that you entered");
            }



            readFile(filepath, false);


            changed = false;



        }


        /// <summary>
        /// helper method that determines if the variable is of valid form of A1 or A_1 
        /// 
        /// </summary>
        /// <param name="item"></param>
        /// <returns>bool</returns>

        private bool variablechecker(string item)
        {
            if (Regex.IsMatch(item, @"^[a-zA-Z]+[\d]+$") && IsValid(item))
                return true;
            else return false;
        }


        /// <summary>
        /// If name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, returns the contents (as opposed to the value) of the named cell.  The return
        /// value should be either a string, a double, or a Formula.
        public override object GetCellContents(string name)
        {


            if (ReferenceEquals(name, null) || !variablechecker(name))
            {
                throw new InvalidNameException();
            }
            name = Normalize(name);
            Cell doubleValue;
            if (spreadsheet.TryGetValue(name, out doubleValue))
            {
                return doubleValue.content;
            }
            else
            {
                return "";
            }








        }



        /// <summary>
        /// Enumerates the names of all the non-empty cells in the spreadsheet.
        /// </summary>
        public override IEnumerable<string> GetNamesOfAllNonemptyCells()
        {
            return spreadsheet.Keys;

        }

        /// <summary>
        /// If name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, the contents of the named cell becomes number.  The method returns a
        /// set consisting of name plus the names of all other cells whose value depends, 
        /// directly or indirectly, on the named cell.
        /// 
        /// For example, if name is A1, B1 contains A1*2, and C1 contains B1+A1, the
        /// set {A1, B1, C1} is returned.
        /// </summary>

        protected override ISet<string> SetCellContents(string name, double number)
        {




            Cell newCell = new Cell(number);
            if (spreadsheet.ContainsKey(name))
            {
                spreadsheet[name] = newCell;
            }
            else
            {
                spreadsheet.Add(name, newCell);
            }


            graph.ReplaceDependees(name, new HashSet<string>());
            HashSet<string> getDependees = new HashSet<string>(GetCellsToRecalculate(name));





            return getDependees;





        }
        /// <summary>
        /// If text is null, throws an ArgumentNullException.
        /// 
        /// Otherwise, if name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, the contents of the named cell becomes text.  The method returns a
        /// set consisting of name plus the names of all other cells whose value depends, 
        /// directly or indirectly, on the named cell.
        /// 
        /// For example, if name is A1, B1 contains A1*2, and C1 contains B1+A1, the
        /// set {A1, B1, C1} is returned.
        /// </summary>
        protected override ISet<string> SetCellContents(string name, string text)
        {






            Cell newcell = new Cell(text);
            if (spreadsheet.ContainsKey(name))
            {

                spreadsheet[name] = newcell;

            }
            else
            {
                spreadsheet.Add(name, newcell);
            }
            if (spreadsheet[name].content.Equals(""))
            {
                spreadsheet.Remove(name);
            }




            graph.ReplaceDependees(text, new HashSet<string>());
            HashSet<string> getDependees = new HashSet<string>(GetCellsToRecalculate(name));


            return getDependees;










        }


        /// <summary>
        /// If the formula parameter is null, throws an ArgumentNullException.
        /// 
        /// Otherwise, if name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, if changing the contents of the named cell to be the formula would cause a 
        /// circular dependency, throws a CircularException.  (No change is made to the spreadsheet.)
        /// 
        /// Otherwise, the contents of the named cell becomes formula.  The method returns a
        /// Set consisting of name plus the names of all other cells whose value depends,
        /// directly or indirectly, on the named cell.
        /// 
        /// For example, if name is A1, B1 contains A1*2, and C1 contains B1+A1, the
        /// set {A1, B1, C1} is returned.
        /// </summary>
        protected override ISet<string> SetCellContents(string name, Formula formula)
        {






            IEnumerable<string> dependeesList1 = graph.GetDependees(name);

            graph.ReplaceDependees(name, formula.GetVariables());


            try
            {
                HashSet<string> dependeeslist2 = new HashSet<string>(GetCellsToRecalculate(name));

                Cell newCell = new Cell(formula, valueOfCell);
                if (spreadsheet.ContainsKey(name))
                {
                    spreadsheet[name] = newCell;

                }
                else
                {
                    spreadsheet.Add(name, newCell);
                }



                return dependeeslist2;

            }
            catch (CircularException)
            {

                graph.ReplaceDependees(name, dependeesList1);
                throw new CircularException();

            }










        }


        /// <summary>
        /// If name is null, throws an ArgumentNullException.
        /// 
        /// Otherwise, if name isn't a valid cell name, throws an InvalidNameException.
        /// 
        /// Otherwise, returns an enumeration, without duplicates, of the names of all cells whose
        /// values depend directly on the value of the named cell.  In other words, returns
        /// an enumeration, without duplicates, of the names of all cells that contain
        /// formulas containing name.
        /// 
        /// For example, suppose that
        /// A1 contains 3
        /// B1 contains the formula A1 * A1
        /// C1 contains the formula B1 + A1
        /// D1 contains the formula B1 - C1
        /// The direct dependents of A1 are B1 and C1
        /// </summary>
        protected override IEnumerable<string> GetDirectDependents(string name)
        {
            if (ReferenceEquals(name, null))
            {
                throw new ArgumentNullException();
            }
            if (!variablechecker(name))
            {
                throw new InvalidNameException();
            }

            return graph.GetDependents(name);




        }
        // ADDED FOR PS5
        /// <summary>
        /// Returns the version information of the spreadsheet saved in the named file.
        /// If there are any problems opening, reading, or closing the file, the method
        /// should throw a SpreadsheetReadWriteException with an explanatory message.
        /// </summary>

        public override string GetSavedVersion(String filename)
        {


            return readFile(filename, true);
        }








        // ADDED FOR PS5
        /// <summary>
        /// Writes the contents of this spreadsheet to the named file using an XML format.
        /// The XML elements should be structured as follows:
        /// 
        /// <spreadsheet version="version information goes here">
        /// 
        /// <cell>
        /// <name>
        /// cell name goes here
        /// </name>
        /// <contents>
        /// cell contents goes here
        /// </contents>    
        /// </cell>
        /// 
        /// </spreadsheet>
        /// 
        /// There should be one cell element for each non-empty cell in the spreadsheet.  
        /// If the cell contains a string, it should be written as the contents.  
        /// If the cell contains a double d, d.ToString() should be written as the contents.  
        /// If the cell contains a Formula f, f.ToString() with "=" prepended should be written as the contents.
        /// 
        /// If there are any problems opening, writing, or closing the file, the method should throw a
        /// SpreadsheetReadWriteException with an explanatory message.
        /// </summary>

        public override void Save(String filename)
        {

            if (ReferenceEquals(filename, null))
            {
                throw new SpreadsheetReadWriteException("filename is null");
            }
            if (filename.Equals(""))
            {
                throw new SpreadsheetReadWriteException("filename is empty");
            }




            try
            {
                XmlWriterSettings setting = new XmlWriterSettings();
                setting.Indent = true;



                using (XmlWriter writer = XmlWriter.Create(filename, setting))
                {
                    writer.WriteStartDocument();
                    writer.WriteStartElement("spreadsheet");
                    writer.WriteAttributeString("version", null, Version);
                    foreach (string cellname in spreadsheet.Keys)
                    {
                        writer.WriteStartElement("cell");
                        writer.WriteElementString("name", cellname);
                        string content;
                        if (spreadsheet[cellname].content is double)
                        {

                            content = spreadsheet[cellname].content.ToString();
                        }
                        else if (spreadsheet[cellname].content is Formula)
                        {
                            content = "=" + spreadsheet[cellname].content.ToString();

                        }
                        else
                        {
                            content = (string)spreadsheet[cellname].content;
                        }
                        writer.WriteElementString("contents", content);
                        writer.WriteEndElement();

                    }
                    writer.WriteEndElement();
                    writer.WriteEndDocument();

                }

            }
            catch (XmlException e)
            {
                throw new SpreadsheetReadWriteException(e.ToString());
            }
            catch (IOException e)
            {
                throw new SpreadsheetReadWriteException(e.ToString());
            }


            changed = false;

        }



        /// <summary>
        /// private helper file that will read the xml file and convert it into a saved spreadsheet 
        /// the readFile method will also 
        /// 
        /// </summary>


        private string readFile(string filename, bool getVersioninfo)
        {
            if (ReferenceEquals(filename, null))
            {
                throw new SpreadsheetReadWriteException("filename is null");
            }
            if (filename.Equals(""))
            {
                throw new SpreadsheetReadWriteException("filename is empty");
            }

            try
            {
                using (XmlReader reader = XmlReader.Create(filename))
                {
                    string name = "";
                    string content = "";
                    while (reader.Read())
                    {
                        if (reader.IsStartElement())
                        {


                            switch (reader.Name)
                            {

                                case "spreadsheet":
                                    if (getVersioninfo)
                                    {
                                        return reader["version"];
                                    }
                                    Version = reader["version"];
                                    break;


                                case "name":
                                    if (getVersioninfo)
                                    {
                                        throw new SpreadsheetReadWriteException("file read error");
                                    }
                                    else
                                    {
                                        reader.Read();
                                        name = reader.Value;


                                    }

                                    break;

                                case "cell":
                                    if (getVersioninfo)
                                    {
                                        throw new SpreadsheetReadWriteException("file read error");
                                    }
                                    break;
                                case "contents":
                                    if (getVersioninfo)
                                    {
                                        throw new SpreadsheetReadWriteException("file read error");
                                    }
                                    reader.Read();
                                    content = reader.Value;
                                    SetContentsOfCell(name, content);


                                    break;

                            }
                        }

                    }
                }
            }
            catch (XmlException e)
            {
                throw new SpreadsheetReadWriteException(e.ToString());
            }
            catch (IOException e)
            {
                throw new SpreadsheetReadWriteException(e.ToString());
            }

            return Version;
        }

        // ADDED FOR PS5
        /// <summary>
        /// If name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, returns the value (as opposed to the contents) of the named cell.  The return
        /// value should be either a string, a double, or a SpreadsheetUtilities.FormulaError.
        /// </summary>
        public override object GetCellValue(String name)
        {
            if (ReferenceEquals(name, null) || !variablechecker(name))
            {
                throw new InvalidNameException();
            }

            Cell item;
            if (spreadsheet.TryGetValue(name, out item))
            {
                return item.value;
            }
            else
            {
                return "";
            }





        }

        public override bool Changed
        {
            get
            {
                return changed;
            }
            protected set
            {
                changed = value;
            }
        }





        // MODIFIED PROTECTION FOR PS5
        /// <summary>
        /// If formula parameter is null, throws an ArgumentNullException.
        /// 
        /// Otherwise, if name is null or invalid, throws an InvalidNameException.
        /// 
        /// Otherwise, if changing the contents of the named cell to be the formula would cause a 
        /// circular dependency, throws a CircularException.
        /// 
        /// Otherwise, the contents of the named cell becomes formula.  The method returns a
        /// Set consisting of name plus the names of all other cells whose value depends,
        /// directly or indirectly, on the named cell.
        /// 
        /// For example, if name is A1, B1 contains A1*2, and C1 contains B1+A1, the
        /// set {A1, B1, C1} is returned.
        /// </summary>
        public override ISet<String> SetContentsOfCell(String name, String content)
        {


            if (ReferenceEquals(content, null))
            {
                throw new ArgumentNullException();
            }
            if (ReferenceEquals(name, null) || !variablechecker(name))
            {
                throw new InvalidNameException();
            }

            HashSet<string> dependentlist;
            double answer;

            if (content.Equals(""))
            {
                dependentlist = new HashSet<string>(SetCellContents(name, content));
            }
            else if (double.TryParse(content, out answer))
            {
                dependentlist = new HashSet<string>(SetCellContents(name, answer));




            }
            else if (content.Substring(0, 1).Equals("="))
            {
                string formulastring = content.Substring(1, content.Length - 1);
                Formula formula = new Formula(formulastring, Normalize, IsValid);
                dependentlist = new HashSet<string>(SetCellContents(name, formula));


            }
            else
            {
                dependentlist = new HashSet<string>(SetCellContents(name, content));
            }





            changed = true;




            foreach (string name2 in dependentlist)
            {
                Cell cell2;
                if (spreadsheet.TryGetValue(name2, out cell2))
                {
                    cell2.Reevaluate(valueOfCell);
                }

            }



            return dependentlist;

        }

        /// <summary>
        /// This is a private class for the object cell which is run in all of the setcellcontents and getcellcontent
        /// It has multiple constructors which allows the cell to instantiate with string name, double value or a formulae.
        /// </summary>

        private class Cell
        {




            public Object content { get; private set; }
            public Object value { get; private set; }
            string contentType = "";


            /// <summary>
            /// 
            /// constructor that allows the cell name to be inputted
            /// </summary>
            /// <param name="name"></param>
            public Cell(string name)
            {

                content = name;
                value = content;
                contentType = "String";



            }

            /// <summary>
            /// 
            /// 
            /// </summary>
            /// <param name="name"></param>
            public Cell(double name)
            {
                content = name;
                value = content;
                contentType = "Double";



            }
            public Cell(Formula name, Func<string, double> lookup)
            {

                content = name;
                value = name.Evaluate(lookup);
                contentType = "Formula";



            }


            /// <summary>
            /// 
            /// This will reevaluate cell if the cell dependent is changed
            /// </summary>
            /// <param name="lookup"></param>
            public void Reevaluate(Func<string, double> lookup)
            {
                if (contentType.Equals("Formula"))
                {
                    Formula formula2 = (Formula)content;
                    value = formula2.Evaluate(lookup);
                }
            }





        }


        private double valueOfCell(string name)
        {
            Cell item;
            name = Normalize(name);
            if (spreadsheet.TryGetValue(name, out item))
            {
                if (item.value is double)
                {
                    return (double)item.value;
                }
                else
                {
                    throw new ArgumentException();
                }
            }
            else
            {
                throw new ArgumentException();
            }

        }







    }






}
