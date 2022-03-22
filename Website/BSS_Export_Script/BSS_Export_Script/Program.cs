using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows.Forms;

namespace BSS_Export_Script
{
    class Program
    {
#if DEBUG
`       //Required
        const string wwwDebugDirectory = @"";
#endif

        const bool ShowDialogs = true;

        /// <summary>
        /// Change var devMode = true to false.
        /// Dev mode is used for working on the website on PC,
        /// mainly loading dummy JSON data.
        /// </summary>
        const bool assureDevModeFalse = true;

        /// <summary>
        /// The data\www directory.
        /// </summary>
        static DirectoryInfo wwwDir;

        // Quick and dirty strings to find the nav element.
        const string navStart = "<nav";
        const string navEnd = "</nav>";

        // Quick and dirty strings to find the footer element.
        const string footerStart = "<footer";
        const string footerEnd = "</footer>";

        // Quick and dirty strings to find the form element.
        const string formStart = "<form";
        const string formEnd = "</form>";

        /// <summary>
        /// When deleting files of type, files with this name will not be deleted.
        /// </summary>
        /// <example>"data.json"</example>
        static string[] keepFiles = new string[] { };

        static string[] deleteFiles = new string[] { "dummyDelete.html" };

        static string[] deleteDirectories = new string[] { "assets\\js\\pcOnly" };


        static void Main(string[] args)
        {
#if DEBUG
            wwwDir = new DirectoryInfo(@"wwwDebugDirectory");
#else

            FileInfo fileInfo = new FileInfo("dummyDelete.html");

            //Get the EXE path.
            DirectoryInfo dir = new FileInfo(System.Reflection.Assembly.GetEntryAssembly().Location).Directory;

            //Find the main project folder.
            while (dir.Name != "ESP_Sense")
            {
                dir = dir.Parent;
            }

            //Set the path to the data\www directory.
            wwwDir = new DirectoryInfo(Path.Combine(dir.FullName, "ESP_Sense\\data\\www"));

#pragma warning disable CS0162 // Unreachable code detected
            if (ShowDialogs)
            {
                DialogResult result = MessageBox.Show("About to delete old files in\r\n" + wwwDir.FullName + "\r\nYes will delete all files in and below the directory.\r\nNo will keep the files, and continue with the script.\r\nCancel will abort running the script.", "Delete Old files?", System.Windows.Forms.MessageBoxButtons.YesNoCancel);

                if (result == DialogResult.Yes)
                {
                    //Delete all of the old files in the www dir.
                    DeleteAllFiles(wwwDir);
                }
                else if (result == DialogResult.Cancel)
                {
                    Console.WriteLine("Export Script Stopped Pre-maturely.");
                    return;
                }
            }
            else
            {
                //Delete all of the old files in the www dir.

                DeleteAllFiles(wwwDir);
            }
#pragma warning restore CS0162 // Unreachable code detected

            //Files have not been exported, or has not been ran from BSS. Exit.
            if (!MoveExportedFiles()) return;
#endif

            FileMatch keepMatch = new FileMatch(keepFiles);
            FileFinder deleteFinder = new FileFinder(wwwDir, deleteFiles);

            string[] deleteDirs = new string[deleteDirectories.Length];

            for(int i = 0; i < deleteDirectories.Length; i++)
            {
                deleteDirs[i] = Path.Combine(wwwDir.FullName, deleteDirectories[i]);
            }

            DirectoryFinder deleteDirFinder = new DirectoryFinder(wwwDir, deleteDirs);

            //Delete all directories maching the names in "deleteDirectories"
            DeleteDirectoriesWithNames(deleteDirFinder);

            //Deletes all files matching the names in "deleteFiles"
            DeleteFilesWithNames(deleteFinder);
            //DeleteFilesWithNames(wwwDir, deleteFiles);

            //Deletes all files of type JSON from the www directory.
            //There are dummy json files for testing the JS code,
            //and the space on the ESP32 needs to be conserved.
            DeleteFilesOfType(wwwDir, ".json", SearchOption.AllDirectories, keepMatch);

            //Execute any changes that are very specific,
            //and code offers no repeatability.
            HardCodedChanges();

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
            DirectoryInfo exportPath = new DirectoryInfo(Path.Combine(Directory.GetCurrentDirectory(), "temp/ESP_Sense/www"));

            DirectoryInfo espSenseTemp = exportPath.Parent;

            //Exe not ran from BSS, files will not be here.
            if (espSenseTemp.Name == "net5.0")
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

#pragma warning disable CS0162 // Unreachable code detected
            if (ShowDialogs)
            {
                DialogResult result = MessageBox.Show("About to move files from\r\n" + exportPath + "\r\nto\r\n" + wwwDir + "\r\nAre these the correct paths?", "Moving exported files?", System.Windows.Forms.MessageBoxButtons.YesNo);

                if (result == DialogResult.Yes)
                {
                    //Move all the files below the exportPath to the data\www directory.
                    MoveAll(exportPath);
                }
                else if (result == DialogResult.No)
                {
                    Console.WriteLine("Export Script Stopped Pre-maturely.");
                    return false;
                }
            }
            else
            {
                //Move all the files below the exportPath to the data\www directory.
                MoveAll(exportPath);
            }
#pragma warning restore CS0162 // Unreachable code detected

            //If there are no remaining files in and below the directory, delete the temporary folder.
            if (espSenseTemp.Name == "ESP_Sense" && espSenseTemp.GetFiles("*", SearchOption.AllDirectories).Length == 0)
                espSenseTemp.Delete(true);

            return true;
        }

        /// <summary>
        /// Deletes all files in the directory dependant on the <paramref name="searchOption"/>
        /// </summary>
        /// <param name="dir"></param>
        /// <param name="searchOption"></param>
        /// <returns></returns>
        static int DeleteAllFiles(DirectoryInfo dir, SearchOption searchOption = SearchOption.AllDirectories)
        {
            var files = dir.EnumerateFiles("*", searchOption).ToList();
            files.ForEach(x => x.Delete());
            return files.Count;
        }

        /// <summary>
        /// Searches for and deletes all files of type in or below a directory.
        /// </summary>
        /// <param name="dirPath">Top directory to start deleting files from.</param>
        /// <param name="fileType">File Extension</param>
        /// <param name="searchOption">Speicifies whether to search the current directory, or the directory and all sub-directories.</param>
        /// <param name="keepFiles">List of filenames you wish to keep. Including the extension.</param>
        static int DeleteFilesOfType(DirectoryInfo dirPath, string fileType, SearchOption searchOption = SearchOption.AllDirectories, FileMatch keepFiles = null)
        {
            FileInfo[] files = dirPath.GetFiles("*" + (fileType.StartsWith(".") ? "" : ".") + fileType, searchOption);

            return DeleteFilesOfType(files, fileType, keepFiles);
        }

        static int DeleteFilesWithNames(FileFinder finder)
        {
            FileInfo[] files = finder.GetMatches().ToArray();

            if (files == null) return -1;

            foreach (FileInfo file in files)
            {
                file.Delete();
            }

            return files.Length;
        }

        /// <summary>
        /// Searches for and deletes all files with a matching name in or below <paramref name="dirPath"/>.
        /// </summary>
        /// <param name="dirPath">Top directory to start deleting files from.</param>
        /// <param name="names">Names to match</param>
        /// <param name="ignoreExtension">If true, as long as the first part of the name is matching the file will be deleted.</param>
        /// <param name="searchOption">Speicifies whether to search the current directory, or the directory and all sub-directories.</param>
        /// <returns>Count of how many files were deleted.</returns>
        static int DeleteFilesWithNames(DirectoryInfo dirPath, string[] names, bool ignoreExtension = false, SearchOption searchOption = SearchOption.AllDirectories)
        {
            FileFinder fileFinder = new FileFinder(dirPath, names, searchOption, ignoreExtension);
            return DeleteFilesWithNames(fileFinder);

            //FileInfo[] files = dirPath.GetFiles("*name*", searchOption);

            //int count = 0;

            //foreach (FileInfo file in files)
            //{
            //    bool match = false;
            //    if (ignoreExtension)
            //    {
            //        foreach (string name in names)
            //        {
            //            string noExtName = String.Concat(name.Split('.', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries));
            //            string noExtFile = String.Concat(file.Name.Split('.', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries));

            //            if (match = noExtName == noExtFile) break;
            //        }
            //    }
            //    else
            //    {
            //        match = names.Contains( file.Name );
            //    }

            //    if (match)
            //    {
            //        file.Delete();
            //        count++;
            //    }
            //}
            //return count;
        }

        /// <summary>
        /// Searches for and deletes all files with a matching name in or below <paramref name="dirPath"/>.
        /// </summary>
        /// <param name="dirPath">Top directory to start deleting files from.</param>
        /// <param name="name">Name to match</param>
        /// <param name="ignoreExtension">If true, as long as the first part of the name is matching the file will be deleted.</param>
        /// <param name="searchOption">Speicifies whether to search the current directory, or the directory and all sub-directories.</param>
        /// <returns>Count of how many files were deleted.</returns>
        static int DeleteFilesWithName(DirectoryInfo dirPath, string name, bool ignoreExtension = false, SearchOption searchOption = SearchOption.AllDirectories)
        {
            return DeleteFilesWithNames(dirPath, new string[] { name }, ignoreExtension, searchOption);
        }

        /// <summary>
        /// Deletes all files that have an extension matching <paramref name="fileType"/>.
        /// </summary>
        /// <param name="files">Array of file paths to search and destroy.</param>
        /// <param name="fileType">File Extension</param>
        /// <param name="keepFilenames">List of filenames you wish to keep. Including the extension.</param>
        static int DeleteFilesOfType(FileInfo[] files, string fileType, FileMatch keepFiles = null)
        {
            int count = 0;
            foreach (FileInfo info in files)
            {
                if (!info.Exists) continue;
                if (!info.Name.EndsWith(fileType)) continue;

                if (keepFiles != null)
                    if (keepFiles.Match(info))
                        continue;

                File.Delete(info.FullName);
                count++;
            }
            return count;
        }

        static int DeleteDirectoriesWithNames(DirectoryFinder finder)
        {
            DirectoryInfo[] directories = finder.GetMatches().ToArray();

            if (directories == null) return -1;

            foreach (DirectoryInfo dir in directories)
            {
                dir.Delete(true);
            }

            return directories.Length;
        }


        static int DeleteDirectoriesWithNames(DirectoryInfo dirPath, string[] names, SearchOption searchOption = SearchOption.AllDirectories)
        {
            DirectoryFinder dirFinder = new DirectoryFinder(dirPath, names, searchOption);
            return DeleteDirectoriesWithNames(dirFinder);
        }


        static int DeleteDirectoriesWithName(DirectoryInfo dirPath, string name, SearchOption searchOption = SearchOption.AllDirectories)
        {
            return DeleteDirectoriesWithNames(dirPath, new string[] { name }, searchOption);
        }



        /// <summary>
        /// Move all files from the temporary export path to the data\www folder.
        /// </summary>
        /// <param name="path">Temporary export path</param>
        static void MoveAll(DirectoryInfo path)
        {
            FileInfo[] files = path.GetFiles("*", SearchOption.AllDirectories);

            foreach (FileInfo file in files)
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

                RemoveElement(filepath, navStart, navEnd, "Nav");
                RemoveElement(filepath, footerStart, footerEnd, "Footer");
                //HideElementsWithClass(filepath, "main-container");
                HideElements(filepath, "main");

                //If page contains partial attribute within the html tag,
                //extract the form 
                CreatePartialPage(filepath, formStart, formEnd);
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

            while (index < file.Length &&  (index = file.IndexOf(cssClass, ++index)) != -1)
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

        /// <summary>
        /// 
        /// </summary>
        /// <param name="path"></param>
        /// <param name="elemType"></param>
        static void HideElements(FileInfo path, string elemType)
        {
            if (!path.Name.EndsWith(".html")) return;

            string file = File.ReadAllText(path.FullName);

            int index = -1;
            int lastIndex = 0;
            int times = 0;

            string addClass = "d-none";

            while (index < file.Length && (index = file.IndexOf("<" + elemType, ++index)) != -1)
            {
                Console.WriteLine("Found elem at index " + index);
                if (index < lastIndex)
                    break;

                const string classString = "class=\"";

                int endElem = file.IndexOf(">", index);
                int classIndex = file.IndexOf(classString, index);
                

                if(endElem < classIndex)
                {
                    file = file.Insert(endElem, " " + classString + addClass + "\"");
                }
                else
                {
                    int endClass = file.IndexOf("\"", classIndex + classString.Length);
                    int insertAt = classIndex + classString.Length;

                    //Already hidden
                    if (file.Substring(insertAt, endClass - insertAt).Contains(addClass))
                        continue;
                    
                    file = file.Insert(insertAt, " " + addClass + " ");
                }

                times++;                
                index += endElem;
                lastIndex = index;
            }

            Console.WriteLine("Done file " + path + "added " + times + "things");
            if (lastIndex > 0)
                File.WriteAllText(path.FullName, file);
        }


        /// <summary>
        /// Warning! Not very safe.
        /// </summary>
        /// <param name="path"></param>
        /// <param name="elemStart"></param>
        /// <param name="elemEnd"></param>
        /// <param name="name"></param>
        static void RemoveElement(FileInfo path, string elemStart, string elemEnd, string name = "Element")
        {
#warning create a remove element which properly parses the HTML
            if (path.Extension != ".html") return;

            string fileContents = File.ReadAllText(path.FullName);

            int start = fileContents.IndexOf(elemStart);
            int end = fileContents.IndexOf(elemEnd);

            if (start != -1 && end != -1)
            {
                fileContents = fileContents.Remove(start, (end + elemEnd.Length) - start);
                Console.WriteLine(name + " Removed");
                File.WriteAllText(path.FullName, fileContents);
            }
        }


        /// <summary>
        /// Extracts one element, and overwrites the page.
        /// </summary>
        /// <param name="path"></param>
        /// <param name="elemStart"></param>
        /// <param name="elemEnd"></param>
        static void CreatePartialPage(FileInfo path, string elemStart, string elemEnd, FileMatch fileMatch = null)
        {
#warning Need to decide what to do with stylesheets.
            if (path.Extension != ".html") return;

            //If filenames are passed, only process files with matching name.
            if (fileMatch != null)
            {
                if (!fileMatch.Match(path)) return;
            }

            string fileContents = File.ReadAllText(path.FullName);

            int htmlStart = fileContents.IndexOf("<html");
            int htmlEnd = fileContents.IndexOf('>', htmlStart);

            //Page is not a partial page. Leave.
            if (fileContents.Substring(htmlStart, htmlEnd - htmlStart).Contains("partial") == false) return;


            int start = fileContents.IndexOf(elemStart);
            int end = fileContents.IndexOf(elemEnd);



            if (start != -1 && end != -1)
            {
                fileContents = fileContents.Substring(start, end - start);
                Console.WriteLine("Partial Page Created : " + path.FullName);
                File.WriteAllText(path.FullName, fileContents);
            }
        }


        static void HardCodedChanges()
        {
            if (assureDevModeFalse)
            {
                var files = wwwDir.EnumerateFiles("esp.js", SearchOption.AllDirectories).ToArray();

                if (files.Length == 1)
                {
                    string contents = File.ReadAllText(files[0].FullName);

                    if (contents.IndexOf("var devMode = true;") != -1)
                    {
                        contents = contents.Replace("var devMode = true;", "var devMode = false;");
                        File.WriteAllText(files[0].FullName, contents);
                    }
                }
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

        public class FileMatch
        {
            protected string[] matchPaths;
            protected DirectoryInfo searchDirectory;

            public SearchOption SearchOption { get; set; }

            public bool IgnoreExtension { get; set; }

            public FileMatch(string filename, bool ignoreExtension = false)
            {
                matchPaths = new string[] { filename };
                IgnoreExtension = ignoreExtension;
            }

            public FileMatch(string[] filenames, bool ignoreExtension = false)
            {
                this.matchPaths = filenames;
                IgnoreExtension = ignoreExtension;
            }

            public FileMatch(DirectoryInfo directory, SearchOption searchOption = SearchOption.AllDirectories)
            {
                this.searchDirectory = directory;
                this.SearchOption = searchOption;
            }

            public FileMatch(DirectoryInfo directory, string filename, SearchOption searchOption = SearchOption.AllDirectories, bool ignoreExtension = false)
            {
                matchPaths = new string[] { filename };
                this.searchDirectory = directory;
                this.SearchOption = searchOption;
                IgnoreExtension = ignoreExtension;
            }

            public FileMatch(DirectoryInfo directory, string[] filenames, SearchOption searchOption = SearchOption.AllDirectories, bool ignoreExtension = false)
            {
                this.matchPaths = filenames;
                this.searchDirectory = directory;
                this.SearchOption = searchOption;
                IgnoreExtension = ignoreExtension;
            }

            virtual public bool Match(FileInfo file)
            {
                bool inDir = true;
                bool nameMatch = true;

                if (searchDirectory != null)
                    if (!InDirectory(searchDirectory, file, "*", SearchOption))
                        inDir = false;

                if (matchPaths != null)
                    if (!MatchFileNames(file, matchPaths, IgnoreExtension))
                        nameMatch = false;

                return inDir && nameMatch;
            }

            public static bool InDirectory(DirectoryInfo directory, FileInfo file, string pattern = "*", SearchOption searchOption = SearchOption.AllDirectories)
            {
                return directory.EnumerateFiles(pattern, searchOption).Any(x => x.FullName == file.FullName);
            }

            static string RemoveExtension(string path)
            {
                int index = path.LastIndexOf('.');

                if (index != -1)
                    path = path.Substring(0, index);

                return path;
            }

            public static bool MatchFileName(FileInfo file, string name, bool ignoreExtension = false)
            {
                if (ignoreExtension)
                    name = RemoveExtension(name);

                if (name.Contains('/') || name.Contains('\\'))
                {
                    if (name == (ignoreExtension ? RemoveExtension(file.FullName) : file.FullName))
                    {
                        return true;
                    }
                }
                else
                {
                    if (name == (ignoreExtension ? RemoveExtension(file.Name) : file.Name))
                    {
                        return true;
                    }
                }
                return false;
            }

            public static bool MatchFileNames(FileInfo file, string[] names, bool ignoreExtension = false)
            {
                foreach (string name in names)
                {
                    if (MatchFileName(file, name, ignoreExtension)) return true;
                }
                return false;
            }
        }


        public class FileFinder : FileMatch
        {

            protected string SearchPattern { get; set; }

            public FileFinder(DirectoryInfo directory, string filename, SearchOption searchOption = SearchOption.AllDirectories, bool ignoreExtension = false, string searchPattern = "*") : base(directory, filename, searchOption, ignoreExtension)
            {
                SearchPattern = searchPattern;
            }

            public FileFinder(DirectoryInfo directory, string[] filenames, SearchOption searchOption = SearchOption.AllDirectories, bool ignoreExtension = false, string searchPattern = "*") : base(directory, filenames, searchOption, ignoreExtension)
            {
                SearchPattern = searchPattern;
            }

            protected bool CanMatch()
            {
                if (searchDirectory == null || matchPaths == null)
                    return false;

                return true;
            }

            virtual public List<FileInfo> GetMatches()
            {
                if (!CanMatch()) return null;
                return searchDirectory.EnumerateFiles(SearchPattern, SearchOption).Where(x => Match(x)).ToList();
            }

            virtual public List<string> GetMatchesFullName()
            {
                if (!CanMatch()) return null;
                IEnumerable<FileInfo> enumerable = searchDirectory.EnumerateFiles(SearchPattern, SearchOption).Where(x => Match(x));
                return enumerable.Select(x => x.FullName).ToList();
            }
        }
    }


    public class DirectoryMatch
    {
        //Reminder to add relative path support.
        protected string[] matchPaths;
        protected DirectoryInfo searchDirectory;


        public SearchOption SearchOption { get; set; }

        public DirectoryMatch(string directoryName)
        {
            matchPaths = new string[] { directoryName };
        }

        public DirectoryMatch(string[] directoryNames)
        {
            this.matchPaths = directoryNames;
        }

        public DirectoryMatch(DirectoryInfo directory, SearchOption searchOption = SearchOption.AllDirectories)
        {
            this.searchDirectory = directory;
            this.SearchOption = searchOption;
        }

        public DirectoryMatch(DirectoryInfo directory, string directoryName, SearchOption searchOption = SearchOption.AllDirectories)
        {
            matchPaths = new string[] { directoryName };
            this.searchDirectory = directory;
            this.SearchOption = searchOption;
        }

        public DirectoryMatch(DirectoryInfo directory, string[] directoryNames, SearchOption searchOption = SearchOption.AllDirectories)
        {
            this.matchPaths = directoryNames;
            this.searchDirectory = directory;
            this.SearchOption = searchOption;
        }

        virtual public bool Match(DirectoryInfo path)
        {
            bool inDir = true;
            bool nameMatch = true;

            if (searchDirectory != null)
                if (!InDirectory(searchDirectory, path, "*", SearchOption))
                    inDir = false;

            if (matchPaths != null)
                if (!MatchDirectoryNames(path, matchPaths))
                    nameMatch = false;

            return inDir && nameMatch;
        }

        public static bool InDirectory(DirectoryInfo directory, DirectoryInfo path, string pattern = "*", SearchOption searchOption = SearchOption.AllDirectories)
        {
            return directory.EnumerateDirectories(pattern, searchOption).Any(x => x.FullName == path.FullName);
        }

        static string RemoveExtension(string path)
        {
            int index = path.LastIndexOf('.');

            if (index != -1)
                path = path.Substring(0, index);

            return path;
        }

        public static bool MathDirectoryName(DirectoryInfo directory, string name)
        {


            if (name == directory.FullName)
            {
                return true;
            }

            return false;
        }

        public static bool MatchDirectoryNames(DirectoryInfo file, string[] names)
        {
            foreach (string name in names)
            {
                if (MathDirectoryName(file, name)) return true;
            }
            return false;
        }
    }


    public class DirectoryFinder : DirectoryMatch
    {

        protected string SearchPattern { get; set; }

        public DirectoryFinder(DirectoryInfo directory, string directoryName, SearchOption searchOption = SearchOption.AllDirectories, string searchPattern = "*") : base(directory, directoryName, searchOption)
        {
            SearchPattern = searchPattern;
        }

        public DirectoryFinder(DirectoryInfo directory, string[] directoryNames, SearchOption searchOption = SearchOption.AllDirectories, string searchPattern = "*") : base(directory, directoryNames, searchOption)
        {
            SearchPattern = searchPattern;
        }

        protected bool CanMatch()
        {
            if (searchDirectory == null || matchPaths == null)
                return false;

            return true;
        }

        virtual public List<DirectoryInfo> GetMatches()
        {
            if (!CanMatch()) return null;
            return searchDirectory.EnumerateDirectories(SearchPattern, SearchOption).Where(x => Match(x)).ToList();
        }

        virtual public List<string> GetMatchesFullName()
        {
            if (!CanMatch()) return null;
            IEnumerable<DirectoryInfo> enumerable = searchDirectory.EnumerateDirectories(SearchPattern, SearchOption).Where(x => Match(x));
            return enumerable.Select(x => x.FullName).ToList();
        }
    }
}

