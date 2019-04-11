using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;


//Author Chenxi Sun Uid u0455173




namespace SpreadsheetGUI
{
 
    class SpreadsheetApplicationContext : ApplicationContext
    {
        
        private int formCount = 0;

       
        private static SpreadsheetApplicationContext appContext;

   
        private SpreadsheetApplicationContext()
        {
        }

   
        public static SpreadsheetApplicationContext getAppContext()
        {
            if (appContext == null)
            {
                appContext = new SpreadsheetApplicationContext();
            }
            return appContext;
        }

   
        public int RunForm(Form form)
        {
          
            formCount++;

           
            form.FormClosed += (o, e) => { if (--formCount <= 0) ExitThread(); };

           
            form.Show();

            return formCount;
        }
    }


    static class Program
    {


        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

         
            SpreadsheetApplicationContext appContext = SpreadsheetApplicationContext.getAppContext();
            appContext.RunForm(new SpreadsheetForm());
            Application.Run(appContext);
        }
    }
}