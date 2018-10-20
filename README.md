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

### Gallery maintainer (still in progress)

Updates images in dumped gallery

	FDumper-cli.exe update [flags] [user 1] [user 2]...
	
supported flags:

* `--all-users`(`-A`) Update all galleries in folder (don't specify a user for this one)

### Additional flags for all modes

* `--api-address`(`-API`) custom API server to use (default is faexport.boothale.net)