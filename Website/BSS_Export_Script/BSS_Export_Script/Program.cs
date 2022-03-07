using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;

namespace BSS_Export_Script
{
    class Program
    {
        /// <summary>
        /// The data\www directory.
        /// </summary>
        static string wwwDir;

        // Quick and dirty strings to find the nav element.
        const string navStart = "<nav";
        const string navEnd = "</nav>";

        static void Main(string[] args)
        {
            //Get the EXE path.
            DirectoryInfo dir = new FileInfo(System.Reflection.Assembly.GetEntryAssembly().Location).Directory;

            //Find the main project folder.
            while(dir.Name != "ESP_Sense")
            {
                dir = dir.Parent;
            }

            //Set the path to the data\www directory.
            wwwDir = Path.Combine(dir.FullName, "Firmware\\data\\www");

            //Files have not been exported, or has not been ran from BSS. Exit.
            if (!MoveExportedFiles()) return;

            //Process all files in and below the data\www directory.
            ProcessAllFilesInAndBelowDirectory(wwwDir);

            Console.WriteLine("All done");
        }

        /// <summary>
        /// Creates the paths required for moving, and executes the move function.
        /// </summary>
        /// <returns>True if the files were moved, false if the files do not exist, or if the <see cref="Directory.GetCurrentDirectory"/> is in the wrong location.</returns>
        static bool MoveExportedFiles()
        {
            string exportPath = Path.Combine(Directory.GetCurrentDirectory(), "temp/ESP_Sense/www");

            DirectoryInfo espSenseTemp = new DirectoryInfo(exportPath).Parent;

            //Exe not ran from BSS, files will not be here.
            if(espSenseTemp.Name == "net5.0")
            {
                Console.WriteLine("Not running from export path.");
                return false;
            }

            //Files have not been exported
            if (!espSenseTemp.Exists)
            {
                Console.WriteLine("Files have not been exported");
                return false;
            }

            //Move all the files below the exportPath to the data\www directory.
            MoveAll(exportPath);

            //If there are no remaining files in and below the directory, delete the temporary folder.
            if (espSenseTemp.Name == "ESP_Sense" && espSenseTemp.GetFiles("*", SearchOption.AllDirectories).Length == 0)
                espSenseTemp.Delete(true);

            return true;
        }

        /// <summary>
        /// Move all files from the temporary export path to the data\www folder.
        /// </summary>
        /// <param name="path">Temporary export path</param>
        static void MoveAll(string path)
        {
            List<string> files = Directory.GetFiles(path, "*", SearchOption.AllDirectories).ToList();

            foreach(string file in files)
            {
                string dest = file.Replace(path, wwwDir);

                FileInfo fileInfo = new FileInfo(file);
                FileInfo destInfo = new FileInfo(dest);

                if (!destInfo.Directory.Exists)
                    destInfo.Directory.Create();

                fileInfo.MoveTo(dest, true);
            }
        }

        /// <summary>
        /// Finds and processes all HTML files in and below <paramref name="dir"/>.
        /// </summary>
        /// <param name="dir">Top directory you wish to start processing from.</param>
        static void ProcessAllFilesInAndBelowDirectory(string dir)
        {
            String[] htmlFiles = Directory.GetFiles(dir, "*.html", SearchOption.AllDirectories);
            Console.Write("Found this many html files " + htmlFiles.Length);
            Console.WriteLine("in " + Environment.CurrentDirectory);
            

            foreach (string filepath in htmlFiles)
            {
                RemoveNav(filepath);
                HideElementsWithClass(filepath, "main-container");
            }
        }

        /// <summary>
        /// Adds the "d-none" css class to all HTML elements containing the <paramref name="cssClass"/>.
        /// </summary>
        /// <param name="path">Path to HTML file.</param>
        /// <param name="cssClass">css class to match.</param>
        static void HideElementsWithClass(string path, string cssClass)
        {
            if (!path.EndsWith(".html")) return;

            string file = File.ReadAllText(path);

            int index = -1;
            int lastIndex = 0;
            int times = 0;

            string addClass = "d-none";

            while ((index = file.IndexOf(cssClass, ++index)) != -1)
            {
                Console.WriteLine("Found class at index " + index);
                if (index < lastIndex)
                    break;

                times++;
                file = file.Insert(index, addClass + " ");
                index += addClass.Length + 2;
                lastIndex = index;
            }

            Console.WriteLine("Done file " + path + "added " + times + "things");
            if (lastIndex > 0)
                File.WriteAllText(path, file);
        }

        /// <summary>
        /// Removes the "nav" element from an HTML file.
        /// </summary>
        /// <remarks>Quick and Dirty</remarks>
        /// <param name="path">Path to HTML file.</param>
        static void RemoveNav(string path)
        {
            if (!path.EndsWith(".html")) return;

            string fileContents = File.ReadAllText(path);

            int start = fileContents.IndexOf(navStart);
            int end = fileContents.IndexOf(navEnd);

            if (start != -1 && end != -1)
            {
                fileContents = fileContents.Remove(start, (end + navEnd.Length) - start);
                Console.WriteLine("Nav Removed");
                File.WriteAllText(path, fileContents);
            }
        }


        //static void RemoveElement(ref string contents, string search)
        //{
        //    //Basically just start at a < and count brackets.
        //    //if the search string exists within those brackets, remove it
        //    //Or could find an HTML parsing library
        //    int startBracketindex = -1;
        //    int lastIndex = 0;
        //    int times = 0;
        //}

    }

}
