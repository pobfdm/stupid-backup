#include <stdio.h>
#include <windows.h>
#include <gio/gio.h>

#include<libintl.h> //gettex
#include<locale.h>	//gettex
#define _(String) gettext (String) //gettex


gchar* homedir, *username, *backupFolder;

char hostname[256];


void copyFromResource(gchar* source, gchar* dest )
{
	GError *error=NULL;
 
	GFile*  mySRC =  g_file_new_for_uri(source);
	GFile*  myDEST =  g_file_new_for_uri(dest);
 
	g_file_copy (mySRC,  myDEST,  G_FILE_COPY_OVERWRITE, NULL, NULL,  NULL,    &error);
 
	if (error!=NULL)
	{
		 g_error(_("Errors on copy resource: %s\n"),error->message);
	} 
}

int initRsync()
{
	
	g_unlink("myBackup.log");
	copyFromResource("resource:///org/stupid-backup/resource/cygcrypto-1.0.0.dll", g_build_filename(g_get_tmp_dir(),"cygcrypto-1.0.0.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cyggcc_s-1.dll", g_build_filename(g_get_tmp_dir(),"cyggcc_s-1.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cygiconv-2.dll", g_build_filename(g_get_tmp_dir(),"cygiconv-2.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cygintl-8.dll", g_build_filename(g_get_tmp_dir(),"cygintl-8.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cygpopt-0.dll", g_build_filename(g_get_tmp_dir(),"cygpopt-0.dll",NULL));
	copyFromResource("resource:///org/stupid-backup/resource/cygssp-0.dll", g_build_filename(g_get_tmp_dir(),"cygssp-0.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cygwin1.dll", g_build_filename(g_get_tmp_dir(),"cygwin1.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/cygz.dll", g_build_filename(g_get_tmp_dir(),"cygz.dll",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/rsync.exe", g_build_filename(g_get_tmp_dir(),"rsync.exe",NULL) );
	copyFromResource("resource:///org/stupid-backup/resource/ssh.exe", g_build_filename(g_get_tmp_dir(),"ssh.exe",NULL ));
	copyFromResource("resource:///org/stupid-backup/resource/ssh-keygen.exe", g_build_filename(g_get_tmp_dir(),"ssh-keygen.exe",NULL ));
	
	
	
}

void runBackup()
{
	printf("Backup running...\n");
	initRsync();
	homedir= g_strdup_printf("/cygdrive/c/Users/%s/", g_get_user_name());
	username= g_strdup_printf("%s",g_get_user_name ());
	gethostname(hostname,256);
	backupFolder=g_strdup_printf("%s-%s-(%s)/","myBackup",username,hostname);
	
	g_mkdir (backupFolder,0777);
	
	g_print("User Home-> %s\n", homedir);
	g_print("User-> %s\n", username);
	
	GError* error=NULL;
	gchar* rsync=g_build_filename(g_get_tmp_dir(),"rsync.exe",NULL);
	gchar* cmd= g_strdup_printf("%s --delete --log-file=myBackup.log --exclude-from='exclusions.txt' -avz \"%s\" \"%s\"   ", rsync,homedir, backupFolder);
	
	system(cmd);
	MessageBox( NULL, _("backup finished"), "Stupid Backup", MB_OK | MB_ICONINFORMATION| MB_TASKMODAL);
}

void initGettex()
{
	gchar* localeFolder=g_build_filename(g_get_tmp_dir(),"stupid-backup-locale",NULL);
    if (!g_file_test(localeFolder,G_FILE_TEST_IS_DIR)) g_mkdir (localeFolder,0777);
    
    //Italian locale
    gchar* FolderLocaleIT=g_build_filename(localeFolder,"it",NULL);
    if (!g_file_test(FolderLocaleIT,G_FILE_TEST_IS_DIR))
    {
		 g_mkdir (FolderLocaleIT,0777);
		 g_mkdir (g_build_filename(FolderLocaleIT,"LC_MESSAGES",NULL),0777); 
		 copyFromResource("resource:///org/stupid-backup/resource/stupid-backup-locale.mo.it",g_build_filename(FolderLocaleIT,"LC_MESSAGES","stupid-backup-locale.mo",NULL) );
	}
    
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
   initGettex();
   
   setlocale(LC_ALL,"");
   bindtextdomain("stupid-backup-locale",g_build_filename(g_get_tmp_dir(),"stupid-backup-locale",NULL) );
   textdomain("stupid-backup-locale");
   
   
   
   if (!g_file_test("exclusions.txt",G_FILE_TEST_EXISTS))
	{
		 FILE * f =g_fopen ("exclusions.txt", "w");
		 g_close(f);
	 }
   
   
   char* text=_("You want to run a full backup of your user?");
    
    int res=MessageBox(NULL, text, "Stupid Backup", MB_YESNO|MB_ICONINFORMATION);
    if(res==IDYES)runBackup();
    
    return 0;
}

