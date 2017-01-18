#include <stdio.h>
#include <windows.h>
#include <gio/gio.h>

#include<libintl.h> //gettex
#include<locale.h>	//gettex
#define _(String) gettext (String) //gettex

#define IDC_MAIN_BUTTON	101			
#define IDC_ABOUT_BUTTON 102
#define IDC_EXIT_BUTTON 103


gchar* homedir, *username, *backupFolder;
char hostname[256];
HWND hwnd;
HBITMAP hBitmap;

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
	gchar* cmd= g_strdup_printf("%s --delete --log-file=myBackup.log --exclude-from=exclusions.txt -avz  \"%s\" \"%s\"   ", rsync,homedir, backupFolder);
	
	system(cmd);
	
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
	}
    copyFromResource("resource:///org/stupid-backup/resource/stupid-backup-locale.mo.it",g_build_filename(FolderLocaleIT,"LC_MESSAGES","stupid-backup-locale.mo",NULL) );
}






LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    

	case WM_COMMAND:
	{
		switch(LOWORD(wparam))
            {
				case IDC_MAIN_BUTTON:
				{
					ShowWindow(hwnd,SW_HIDE);
					
					char* text=_("You want to run a full backup of your user?");
					int res=MessageBox(hwnd, text, "Stupid Backup", MB_YESNO|MB_ICONINFORMATION);
					if(res==IDYES)
					{
						runBackup();
						MessageBox( NULL, _("backup finished"), "Stupid Backup", MB_OK | MB_ICONINFORMATION| MB_TASKMODAL);
						ShowWindow(hwnd,SW_SHOW);
						ShowWindow(hwnd,SW_RESTORE);
					}else{
						ShowWindow(hwnd,SW_SHOW);
						ShowWindow(hwnd,SW_RESTORE);
					}
					
					
				}
				break;
				
				case IDC_ABOUT_BUTTON:
				{
					 MessageBox( hwnd, _("Copyright 2017 \n Fabio Di Matteo (pobfdm@gmail.com) \n \n www.freemedialab.org"), "Stupid Backup", MB_OK | MB_ICONINFORMATION| MB_TASKMODAL);
				}
				break;
				
				case IDC_EXIT_BUTTON:
				{
					PostQuitMessage( 0);
				}
				break;
			}
			break;	
	}


    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
} 


int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
	initGettex();

	setlocale(LC_ALL,"");
	bindtextdomain("stupid-backup-locale",g_build_filename(g_get_tmp_dir(),"stupid-backup-locale",NULL) );
	textdomain("stupid-backup-locale");
	
	
	if (!g_file_test("exclusions.txt",G_FILE_TEST_EXISTS))
	{
		//g_file_set_contents("exclusions.txt", " ", -1, NULL);
		system ("echo. 2>exclusions.txt");
	}
   
   
	
	//main win
	char *AppTitle="Stupid Backup";
	WNDCLASS wc;
	
	MSG msg;

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=WindowProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInst;
	wc.hIcon=LoadIcon(NULL,IDI_WINLOGO);
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME;
	wc.lpszMenuName=NULL;
	wc.lpszClassName=AppTitle;
	wc.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(1) );

	if (!RegisterClass(&wc))
	return 0;

	hwnd = CreateWindow(AppTitle,AppTitle,
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT,CW_USEDEFAULT,320,200,
	NULL,NULL,hInst,NULL);
	
	//disable maximize button
	SetWindowLong(hwnd, GWL_STYLE,
               GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
	
	// Create "Run backup" button
			HWND hWndButton0=CreateWindowEx(NULL,
				"BUTTON",
				_("Run backup"),
				WS_TABSTOP|WS_VISIBLE|
				WS_CHILD|BS_DEFPUSHBUTTON,
				100,
				50,
				120,
				24,
				hwnd,
				(HMENU)IDC_MAIN_BUTTON,
				GetModuleHandle(NULL),
				NULL);
	// Create "About" button
			HWND hWndButton1=CreateWindowEx(NULL,
				"BUTTON",
				_("About..."),
				WS_TABSTOP|WS_VISIBLE|
				WS_CHILD|BS_DEFPUSHBUTTON,
				100,
				80,
				120,
				24,
				hwnd,
				(HMENU)IDC_ABOUT_BUTTON,
				GetModuleHandle(NULL),
				NULL);
				
	// Create "Exit" button
			HWND hWndButton2=CreateWindowEx(NULL,
				"BUTTON",
				_("Exit"),
				WS_TABSTOP|WS_VISIBLE|
				WS_CHILD|BS_DEFPUSHBUTTON,
				100,
				110,
				120,
				24,
				hwnd,
				(HMENU)IDC_EXIT_BUTTON,
				GetModuleHandle(NULL),
				NULL);			
	
	
	if (!hwnd)
	return 0;

	ShowWindow(hwnd,nShowCmd);
	UpdateWindow(hwnd);

	while (GetMessage(&msg,NULL,0,0) > 0)
	{
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	}
	
	return 0;
}

