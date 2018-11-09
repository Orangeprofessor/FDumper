# FDumper

### FurAffinity gallery downloader & maintainer using FAExport

### Questions or problems? Contact me on twitter: https://twitter.com/Orangeprofessor

Base flags:
* `--api-address` Web address for the server running FAExport (faexport.boothale.net by default, most users can ignore this flag)

## Main modes

### Gallery dumper

Dumps full gallery to disk

	dump [flags] [user 1] [user 2]...

supported flags:

* `--sfw-only` Only dump SFW submissions.
* `--nsfw-only` Only dump NSFW submissions.
* `--scraps-only` Only dump images from the scraps folder.
* `--no-scraps` Only dump images from the main gallery.

Specifying no content filtering flags will dump all submissions

### Gallery maintainer 

Updates images in dumped gallery

	update [flags]  [user 1] [user 2]...
	
supported flags:

* `--all-users` Update all galleries in folder (don't specify a user for this one)


## Gallery dump tutorial

* Download the latest version of FDumper-cli from the releases section
* Launch it
* Use the 'dump' mode, a dialog box will appear asking for a folder to dump the gallery in
* Patiently wait while FDumper-cli downloads your furry art
 
Example commands:
~~~text
FDumper> dump -sfw --scraps-only strange-fox
~~~

a dialog box will appear after the above command was issued
after a folder is selected, the program will start downloading the gallery

Example output:
~~~text
FDumper> dump --sfw-only --scraps-only strange-fox
Processing user gallery 'strange-fox'
Dumping gallery 'strange-fox' to 'C:\Users\Orangeprofessor\Desktop\CoolArtists\strange-fox'
Downloading scrap submission pages...Done!
23 total images
Downloading scrap submission data...[=============================================] 100%
Downloading submissions...[=============================================] 100% (72134-1142273564.stra...)
Finished processing user gallery 'strange-fox'
~~~
  

## Gallery update tutorial

* Download the latest version of FDumper-cli from the releases section
* Launch it
* Use the 'update' mode, a dialog box will appear asking for a folder, choose the folder containing the folder of the dumped gallery 
* Patiently wait while FDumper-cli downloads your furry art

Example commands:
~~~text
FDumper> update strange-fox
~~~

a dialog box will appear after the above command was issued
select the folder containing the folder of the dumped gallery, for example:
~~~text
select this --> C:\Users\Orangeprofessor\Desktop\CoolArtists\
not this --> C:\Users\Orangeprofessor\Desktop\CoolArtists\strange-fox
~~~

Example output:
~~~text
(This is in the same context as above, so I selected the 'CoolArtists' folder)
FDumper> update strange-fox
Processing gallery 'strange-fox'
Downloading new scrap submission pages...Done!
23 total images
Scanning images in scrap folder for strange-fox...Done!
Comparing new and legacy submission lists...Done!
3 submission(s) not found!
Downloading new scrap submission data...[=============================================] 100%
Downloading new submissions...[=============================================] 100% (7411290-1329367100.st...)
Finished processing gallery 'strange-fox'
~~~

