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
        static DirectoryInfo wwwDir;

        // Quick and dirty strings to find the nav element.
        const string navStart = "<nav";
        const string navEnd = "</nav>";

        // Quick and dirty strings to find the nav element.
        const string footerStart = "<footer";
        const string footerEnd = "</footer>";

        /// <summary>
        /// When deleting files of type, files with this name will not be deleted.
        /// </summary>
        /// <example>"data.json"</example>
        static string[] keepFiles = new string[] { };

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
            wwwDir = new DirectoryInfo( Path.Combine(dir.FullName, "ESP_Sense\\data\\www"));

            //Files have not been exported, or has not been ran from BSS. Exit.
            if (!MoveExportedFiles()) return;

            //Deletes all files of type JSON from the www directory.
            //There are dummy json files for testing the JS code,
            //and the space on the ESP32 needs to be conserved.
            DeleteFilesOfType(wwwDir, ".json", SearchOption.AllDirectories, keepFiles);

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
            DirectoryInfo exportPath = new DirectoryInfo( Path.Combine(Directory.GetCurrentDirectory(), "temp/ESP_Sense/www"));

            DirectoryInfo espSenseTemp = exportPath.Parent;

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
        /// Searches for and deletes all files of type in or below a directory.
        /// </summary>
        /// <param name="dirPath">Top directory to start deleting files from.</param>
        /// <param name="fileType">File Extension</param>
        /// <param name="searchOption">Speicifies whether to search the current directory, or the directory and all sub-directories.</param>
        /// <param name="keepFilenames">List of filenames you wish to keep. Including the extension.</param>
        static void DeleteFilesOfType(DirectoryInfo dirPath, string fileType, SearchOption searchOption = SearchOption.AllDirectories, params string[] keepFilenames)
        {
            FileInfo[] files = dirPath.GetFiles("*" + (fileType.StartsWith(".") ? "" : ".") + fileType, searchOption);

            DeleteFilesOfType(files, fileType, keepFilenames);
        }

        /// <summary>
        /// Deletes all files that have an extension matching <paramref name="fileType"/>.
        /// </summary>
        /// <param name="files">Array of file paths to search and destroy.</param>
        /// <param name="fileType">File Extension</param>
        /// <param name="keepFilenames">List of filenames you wish to keep. Including the extension.</param>
        static void DeleteFilesOfType(FileInfo[] files, string fileType, params string[] keepFilenames)
        {
            foreach (FileInfo info in files)
            {
                if (!info.Exists) continue;
                if (!info.Name.EndsWith(fileType)) continue;

                if (keepFilenames != null && keepFilenames.Length > 0)
                    if (keepFilenames.Contains(info.Name))
                        continue;

                File.Delete(info.FullName);
            }
        }

        /// <summary>
        /// Move all files from the temporary export path to the data\www folder.
        /// </summary>
        /// <param name="path">Temporary export path</param>
        static void MoveAll(DirectoryInfo path)
        {
            FileInfo[] files = path.GetFiles("*", SearchOption.AllDirectories);

            foreach(FileInfo file in files)
            {
                if (!file.Exists) continue;

                string dest = file.FullName.Replace(path.FullName, wwwDir.FullName);

                FileInfo destInfo = new FileInfo(dest);

                if (!destInfo.Directory.Exists)
                    destInfo.Directory.Create();

                file.MoveTo(dest, true);
            }
        }

        /// <summary>
        /// Finds and processes all HTML files in and below <paramref name="dir"/>.
        /// </summary>
        /// <param name="dir">Top directory you wish to start processing from.</param>
        static void ProcessAllFilesInAndBelowDirectory(DirectoryInfo dir)
        {
            FileInfo[] htmlFiles = dir.GetFiles("*.html", SearchOption.AllDirectories);
            Console.Write("Found this many html files " + htmlFiles.Length);
            Console.WriteLine("in " + Environment.CurrentDirectory);
            

            foreach (FileInfo filepath in htmlFiles)
            {
                if (!filepath.Exists) continue;

                RemoveNav(filepath);
                RemoveFooter(filepath);
                HideElementsWithClass(filepath, "main-container");
            }
        }

        /// <summary>
        /// Adds the "d-none" css class to all HTML elements containing the <paramref name="cssClass"/>.
        /// </summary>
        /// <param name="path">Path to HTML file.</param>
        /// <param name="cssClass">css class to match.</param>
        static void HideElementsWithClass(FileInfo path, string cssClass)
        {
            if (!path.Name.EndsWith(".html")) return;

            string file = File.ReadAllText(path.FullName);

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
                File.WriteAllText(path.FullName, file);
        }

#warning create a remove element function and pass nav and footer to.

        /// <summary>
        /// Removes the "nav" element from an HTML file.
        /// </summary>
        /// <remarks>Quick and Dirty</remarks>
        /// <param name="path">Path to HTML file.</param>
        static void RemoveNav(FileInfo path)
        {
            if (!path.Name.EndsWith(".html")) return;

            string fileContents = File.ReadAllText(path.FullName);

            int start = fileContents.IndexOf(navStart);
            int end = fileContents.IndexOf(navEnd);

            if (start != -1 && end != -1)
            {
                fileContents = fileContents.Remove(start, (end + navEnd.Length) - start);
                Console.WriteLine("Nav Removed");
                File.WriteAllText(path.FullName, fileContents);
            }
        }

        /// <summary>
        /// Removes the "nav" element from an HTML file.
        /// </summary>
        /// <remarks>Quick and Dirty</remarks>
        /// <param name="path">Path to HTML file.</param>
        static void RemoveFooter(FileInfo path)
        {
            if (!path.Name.EndsWith(".html")) return;

            string fileContents = File.ReadAllText(path.FullName);

            int start = fileContents.IndexOf(footerStart);
            int end = fileContents.IndexOf(footerEnd);

            if (start != -1 && end != -1)
            {
                fileContents = fileContents.Remove(start, (end + footerEnd.Length) - start);
                Console.WriteLine("Footer Removed");
                File.WriteAllText(path.FullName, fileContents);
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
