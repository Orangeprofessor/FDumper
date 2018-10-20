# FDumper

### FurAffinity gallery downloader & maintainer using FAExport

### Questions or problems? Contact me on twitter: https://twitter.com/Orangeprofessor

## Main modes

### Gallery dumper

Dumps full gallery to disk

	FDumper-cli.exe dump [flags] [user 1] [user 2]...

supported flags:

* `--sfw-only`(`-sfw`) Only dump SFW submissions.
* `--nsfw-only`(`-nsfw`) Only dump NSFW submissions.
* `--scraps-only`(`-scraps`) Only dump images from the scraps folder.
* `--no-scraps`(`-noscraps`) Only dump images from the main gallery.

### Gallery maintainer 

Updates images in dumped gallery

	FDumper-cli.exe update [flags] [user 1] [user 2]...
	
supported flags:

* `--all-users`(`-A`) Update all galleries in folder (don't specify a user for this one)


## Gallery dump tutorial

* Download the latest version of FDumper-cli from the releases section
* Open command prompt and navigate to where FDumper-cli is downloaded
* Use the 'dump' mode, a dialog box will appear asking for a folder to dump the gallery in
* Patiently wait while FDumper-cli downloads your furry art
 
Example commands:
~~~text
cd C:\Users\Orangeprofessor\Downloads
FDumper-cli.exe dump -sfw -scraps strange-fox
(a dialog box will appear after the above command was issued)
(after a folder is selected, the program will start downloading the gallery)
~~~

Example output:
~~~text
[FDumper] Processing user gallery 'strange-fox'
[FDumper] Dumping gallery 'strange-fox' to 'C:\Users\Orangeprofessor\Desktop\CoolArtists\strange-fox'
[FDumper] Downloading scrap submission pages...Done!
[FDumper] 23 total images
[FDumper] Downloading scrap submission data...[=============================================] 100%
[FDumper] Downloading submissions...[=============================================] 100% (72134-1142273564.stra...)
[FDumper] Finished processing user gallery 'strange-fox'
~~~
  

## Gallery update tutorial

* Download the latest version of FDumper-cli from the releases section
* Open command prompt and navigate to where FDumper-cli is downloaded
* Use the 'update' mode, a dialog box will appear asking for a folder, choose the folder containing the folder of the dumped gallery 
* Patiently wait while FDumper-cli downloads your furry art

Example output:
~~~text
(This is in the same context as above, so I selected the 'CoolArtists' folder)
[FDumper] Processing gallery 'strange-fox'
[FDumper] Downloading new scrap submission pages...Done!
[FDumper] 23 total images
[FDumper] Scanning images in scrap folder for strange-fox...Done!
[FDumper] Comparing new and legacy submission lists...Done!
[FDumper] 3 submission(s) not found!
[FDumper] Downloading new scrap submission data...[=============================================] 100%
[FDumper] Downloading new submissions...[=============================================] 100% (7411290-1329367100.st...)
[FDumper] Finised processing gallery 'strange-fox'
~~~


