
<img src="resource/icon.png" height="126" width="126" >

# Stupid Backup
A simple and almost stupid windows application to do incremental backups
of your data.

Put the executable (stupid-backup.exe) in the folder (or flash drive, or
hard disk) where the backups should be stored,double click on it and 
... done!



[**Download**](http://www.freemedialab.org/listing/stupid-backup/)

## Advanced uses

You can also manage your exclusions on exclusions.txt files generated at
the first start by following the syntax of rsync.

If you need to include custom sources paths, you can create the 
"sources.txt file" to include paths in the following form:

	/c/Program Files/important/
	/d/myData/
	/f/photos/

You can also choose a different destination for backups by creating the 
"destination.txt" with the path to the destination folder, for example:	

	/c/Users/Admin/Desktop/custom/
