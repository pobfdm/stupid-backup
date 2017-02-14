#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <gio/gio.h>


#include<libintl.h> //gettex
#include<locale.h>	//gettex
#define _(String) gettext (String) //gettex

#define IDC_MAIN_BUTTON	101			
#define IDC_ABOUT_BUTTON 102
#define IDC_EXIT_BUTTON 103
#define IDC_CHECK_SHUTDOWN 104


gchar* homedir, *username, *backupFolder;
char hostname[256];
HWND hwnd;
HWND hWndButton0;

HWND hwndPB, hWndCheckShutdown ;
gboolean chkProgress=TRUE;
int p=0;
FILE * outFile;
STARTUPINFO si;
PROCESS_INFORMATION pi;
BOOL checkedShutdown;


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

int GetPrivileges()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
 
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
	  wprintf(L"Error on Privileges ...\n");			
      return( FALSE ); 
	}
   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
 
   tkp.PrivilegeCount = 1;      
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
 
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
 
   if (GetLastError() != ERROR_SUCCESS)
   { 
     wprintf(L"Error on Privileges ...\n");
      return FALSE; 
	}
}
int Poweroff()
{
	GetPrivileges();
 
	if (!ExitWindowsEx(EWX_POWEROFF, 0) )
    {
		wprintf(L"Error on shutdown \n");
		return FALSE ;             
	}else{
		wprintf(L"Shutdown...\n");
	}
}


void runBackup()
{
	
	//Gui
	ShowWindow(hwnd,SW_SHOW);
	SwitchToThisWindow(hwnd, TRUE);
	EnableWindow(hWndButton0, FALSE);
	ShowWindow(hwndPB, SW_NORMAL);
	SendMessage( hwndPB, PBM_SETMARQUEE, TRUE, 0 );
	
	
	printf("Backup running...\n");
	chkProgress=TRUE;
	
	initRsync();
	homedir= g_strdup_printf("/cygdrive/c/Users/%s/", g_get_user_name());
	username= g_strdup_printf("%s",g_get_user_name ());
	gethostname(hostname,256);
	
	if (!g_file_test("destination.txt",G_FILE_TEST_EXISTS))
	{
		backupFolder=g_strdup_printf("%s-%s-(%s)/","myBackup",username,hostname);
		g_mkdir (backupFolder,0777);
	}else{
		gchar** dest;
		g_file_get_contents("destination.txt",&dest,NULL,NULL);
		backupFolder=g_strdup_printf("/cygdrive/%s",dest);
	}
	
	g_print("User Home-> %s\n", homedir);
	g_print("User-> %s\n", username);
	
	GError* error=NULL;
	gchar* rsync=g_build_filename(g_get_tmp_dir(),"rsync.exe",NULL);
	gchar *cmd;
	
	
	if (!g_file_test("sources.txt",G_FILE_TEST_EXISTS))
	{
		cmd= g_strdup_printf("%s --delete  --log-file=myBackup.log --info=progress2 --exclude-from=exclusions.txt -avzr --perms --chmod=a=rw,Da+x \"%s\" \"%s\"  ", rsync,homedir, backupFolder);
	}else{
		cmd= g_strdup_printf("%s --delete  --log-file=myBackup.log --info=progress2 --exclude-from=exclusions.txt -avzr --perms --chmod=a=rw,Da+x --files-from=sources.txt  /cygdrive/  \"%s\"  ", rsync, backupFolder);
	}
	
	if (g_file_test("cmdline.txt",G_FILE_TEST_EXISTS))
	{
		gchar *cmdline;
		g_file_get_contents("cmdline.txt",&cmdline,NULL,NULL);
		cmd=g_strdup_printf(cmdline, rsync);
	}
		
	if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, NULL , NULL, NULL, &si, &pi))
	{
		
		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );
		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}else {
		MessageBox( NULL, GetLastError(), _("Warning"), MB_OK | MB_ICONERROR| MB_TASKMODAL);
	}

      
	
	chkProgress=FALSE;
	
	
	//gui
	EnableWindow(hWndButton0, TRUE);
	SendMessage( hwndPB, PBM_SETMARQUEE, FALSE, 0 );
	SendMessage(hwndPB, PBM_SETPOS, 100, 0);
	if (checkedShutdown) //Shutdown or not?
	{
		Poweroff();
	}else{
		MessageBox( hwnd, _("backup finished"), "Stupid Backup", MB_OK | MB_ICONINFORMATION| MB_TASKMODAL);
	}
	SendMessage(hwndPB, PBM_SETPOS, 0, 0);
	ShowWindow(hwndPB, SW_HIDE);
	
	
	
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

void killRsync()
{
	STARTUPINFO info={sizeof(info)};
		PROCESS_INFORMATION processInfo;
		if (CreateProcess(NULL, "taskkill  /f /im rsync.exe", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &processInfo))
		{
			WaitForSingleObject( pi.hProcess, INFINITE );
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}
}

void checkProgress(){
		//you have any idea about it?
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
					//ShowWindow(hwnd,SW_HIDE);
					char* text=_("You want to run a full backup of your user?");
					int res=MessageBox(hwnd, text, "Stupid Backup", MB_YESNO|MB_ICONINFORMATION);
					if(res==IDYES)
					{
						//Check progress
						CreateThread( 
								NULL,                   // default security attributes
								0,                      // use default stack size  
								checkProgress,       // thread function name
								NULL,          // argument to thread function 
								0,                      // use default creation flags 
								NULL);
						
						//run backup
						CreateThread( 
								NULL,                   // default security attributes
								0,                      // use default stack size  
								runBackup,       // thread function name
								NULL,          // argument to thread function 
								0,                      // use default creation flags 
								NULL);		
								
								
						
						
					}else{
						SwitchToThisWindow(hwnd, TRUE);
						ShowWindow(hwnd,SW_SHOW);
					}
					
					
				}
				break;
				
				case IDC_ABOUT_BUTTON:
				{
					 MessageBox( hwnd, _("Copyright 2017 \n Fabio Di Matteo (pobfdm@gmail.com) \n \n www.freemedialab.org"), "Stupid Backup", MB_OK | MB_ICONINFORMATION| MB_TASKMODAL);
				}
				break;
				
				case IDC_CHECK_SHUTDOWN:
				{
					checkedShutdown = IsDlgButtonChecked(hwnd, IDC_CHECK_SHUTDOWN);
				}
				break;
				
				case IDC_EXIT_BUTTON:
				{
					killRsync();
					WindowProc(hwnd, WM_CLOSE,NULL,NULL);
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );
					exit(0);
				}
				break;
			}
			break;	
	}


    case WM_DESTROY:
		//PostQuitMessage(0);
		killRsync();
		WindowProc(hwnd, WM_CLOSE,NULL,NULL);
		exit(0);	
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
		FILE *f0 = fopen("exclusions.txt", "w");
		if (f0 == NULL)
		{
			printf(_("Error on create exclusions.txt file!\n"));
		}
		fclose(f0);
	}
   /*if (!g_file_test("sources.txt",G_FILE_TEST_EXISTS))
	{
		FILE *f1 = fopen("sources.txt", "w");
		if (f1 == NULL)
		{
			printf(_("Error on create sources.txt file!\n"));
		}
		fclose(f1);
	}*/
   
	
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
	//wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=AppTitle;
	wc.hIcon = LoadIcon( hInst, MAKEINTRESOURCE(1) );

	if (!RegisterClass(&wc))
	return 0;

	hwnd = CreateWindow(AppTitle,AppTitle,
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT,CW_USEDEFAULT,320,260,
	NULL,NULL,hInst,NULL);
	
	//disable maximize button
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
	
	
	// Create "Run backup" button
			hWndButton0=CreateWindowEx(NULL,
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
	
	//Create checkbox for shutdown
	hWndCheckShutdown = CreateWindow("BUTTON", _("Poweroff at end"),
                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                 100, 140, 185, 35,        
                 hwnd, (HMENU) IDC_CHECK_SHUTDOWN, GetModuleHandle(NULL), NULL);
    CheckDlgButton(hwnd,  IDC_CHECK_SHUTDOWN, BST_UNCHECKED);             
	
	
	// Create progressbar
			hwndPB = CreateWindowEx(
				0, PROGRESS_CLASS, (LPCWSTR)NULL,
				WS_CHILD | WS_VISIBLE| PBS_MARQUEE,
				100, 190, 120, 25,
				hwnd, (HMENU) 0, hInst, NULL);
				ShowWindow(hwndPB, SW_HIDE);
									
	
	
	
	
	
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

