#include <ctype.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <pwd.h>
#include <grp.h>
#include <fstream>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <cstring>  



//b
#define cursorforward(x) printf("\033[%dC", (x))
#define cursorbackward(x) printf("\033[%dD", (x))
#define cursorup(x) printf("\033[%dA", (x))
#define cursordown(x) printf("\033[%dB", (x))

#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108
//e
#define CTRL_KEY(k) ((k) & 0x1f)
using namespace std;

//---------to implement Go back and Go Forward--------------- 
stack<string> prevstack;
stack<string> forwstack;
//curr dir path
vector<string> upstack;
//present directory
string present="";
bool flag=0;
int k=0, l=22;

char current[4096]; 
char root[4096]; 
vector<string> dirList; 
unsigned int totaldir;
int x = 0, y = 0;
int cursorTracker = 0;

//er beg
struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};
struct editorConfig E;
//er end



//b
static struct termios term, oterm;

static int getch(void);
static int kbhit(void);
static int kbesc(void);
static int kbget(void);

static int getch(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

static int kbhit(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    if (c != -1) ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

static int kbesc(void)
{
    int c;

    if (!kbhit()) return KEY_ESCAPE;
    c = getch();
    if (c == '[') {
        switch (getch()) {
            case 'A':
                c = KEY_UP;
                break;
            case 'B':
                c = KEY_DOWN;
                break;
            case 'C':
                c = KEY_LEFT;
                break;
            case 'D':
                c = KEY_RIGHT;
                break;
            case 13:
                c = KEY_ENTER;
                break;
            default:
                c = 0;
                break;
        }
    } else {
        c = 0;
    }
    if (c == 0) while (kbhit()) getch();
    return c;
}

static int kbget(void)
{
    int c;

    c = getch();
    return (c == KEY_ESCAPE) ? kbesc() : c;
}
//e

//-------------- To find if path is Dirctory or File----------------
bool isDirectory(string path){
   

    struct stat statsD;
     stat(path.c_str(), &statsD);

     return ((statsD.st_mode & S_IFMT) == S_IFDIR)?1:0;
}
bool isRegularFile(const char*path){
   

    struct stat statsF;
     stat(path, &statsF);

     return ((statsF.st_mode & S_IFMT) == S_IFREG)?1:0;
}


//---------------------------------Display files--------------------------------------------------
// ListDirectory -->  displayDir() --> fileProperties

void fileProperties( const char *filepath ){
    struct stat stats;
    struct passwd *pwdd; 
    struct group *grp;
    int i;
    stat( filepath, &stats );
    string s = filepath;
    size_t pos;
    pos = s.find_last_of('/');
    string name1 = s.substr( pos+1, s.length() );
    string name = "";
    for( i=0 ; i<20 && i<name1.size() ; i++){
        if( i<=16 )
            name += name1[i];
        else
            name += '.';
    }
    char temp[500];
    strcpy(temp, name.c_str());
    //cout<<name<<"\t";
    printf("%-20s\t", name.c_str());
    cout<<( (S_ISDIR(stats.st_mode)) ? "d" : "-");
    cout<<( (stats.st_mode & S_IRUSR) ? "r" : "-");
    cout<<( (stats.st_mode & S_IWUSR) ? "w" : "-");
    cout<<( (stats.st_mode & S_IXUSR) ? "x" : "-");
    cout<<( (stats.st_mode & S_IRGRP) ? "r" : "-");
    cout<<( (stats.st_mode & S_IWGRP) ? "w" : "-");
    cout<<( (stats.st_mode & S_IXGRP) ? "x" : "-");
    cout<<( (stats.st_mode & S_IROTH) ? "r" : "-");
    cout<<( (stats.st_mode & S_IWOTH) ? "w" : "-");

    cout<<"\t";

    pwdd = getpwuid(stats.st_uid);
    grp = getgrgid(stats.st_gid);
    if( pwdd != 0 )
        printf("%-8s", pwdd->pw_name);
    if( grp != 0 )
        printf(" %-8s\t", grp->gr_name);
    long long size = stats.st_size;
    if (size >= (1 << 30))
        printf("%4lldG\t", size / (1 << 30));
    else if (size >= (1 << 20))
        printf("%4lldM\t", size / (1 << 20));
    else if (size >= (1 << 10))
        printf("%4lldK\t", size / (1 << 10));
    else
        printf("%4lldB\t", size);

    char *ch = ctime( &stats.st_mtime );
    ch[strlen(ch) - 1] = '\0';
    printf("%30s", ch);

    cout<<"\n";
}

void displayDir(){
    totaldir = dirList.size();
    unsigned int i=0;
    for( i=k ; i<totaldir && i<=l ; i++ ){
        string file = dirList[i]; 
        fileProperties(file.c_str());
    }
}

void ListDirectory( const char* path ){
    dirList.clear();
    struct dirent *d;
    DIR *curdir;
    curdir = opendir(path);
    if(curdir == NULL){
        perror("opendir");
        return;
    }
    while ( ( d = readdir(curdir) ) ) {    
        if( string(d->d_name) == "." ){
            //printf("%-20s\t", d->d_name);
            //if( k<1 )
            //    fileProperties(d->d_name);
            dirList.push_back(string(d->d_name));
            continue;
        }
        if ((string(d->d_name) == "..") && (strcmp(path, root) == 0)){
                //printf("%-20s\t", d->d_name);
                //if( k<2 )
                //    fileProperties(d->d_name);
                dirList.push_back(string(d->d_name));
                continue;
        }
        else{
            dirList.push_back(string(d->d_name));//pushing each file in the directory list        
        }
    }
    sort(dirList.begin(), dirList.end());
    displayDir();
    closedir(curdir);
}


//er beg
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);
  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(ICRNL | IXON);
  //raw.c_oflag &= ~(OPOST); convert \r\n to \n
  //raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP ); misc - outdated flag - useless
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
//er end

//---------------------Command Mode--------------------------

int cnum;
//-------------------------- CREATE FILE -----------------------------------------
void createFile( vector<string> word ){
    int t = word.size();
    int i = 1;
    char *dest;
    string dpath;
    string des;
    if( word[t-1] == "." ){// Current directory
        dpath = current;
    }
    else{
        string dp = word[t-1];
        if( dp[0] == '/' ){// Absolute path from /home/jasika/...
            dpath = word[t-1];
        }
        else{// relative path 
            string rp(root);
            dpath = root + word[t-1].substr(1);
        }
    }
    while( i < t-1 ){
        des = dpath + "/" + word[i];
        dest = new char[des.length()+1];
        strcpy(dest, des.c_str());
        int fd1 = open(dest ,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );  
        if( fd1 < 0 ){
            cout<<"\nError in create_file command";
            break;
        }
        i++;
    }
}

//-------------------------- CREATE DIRECTORY -----------------------------------------
void createDir( vector<string> word ){
    int t = word.size();
    int i = 1;
    char *dest;
    string dpath;
    string des;
    if( word[t-1] == "." ){//current directory
        dpath = current;
    }
    else{
        string dp = word[t-1];
        if( dp[0] == '/' ){// Absolute path from /home/jasika/...
            dpath = word[t-1];
        }
        else{// relative path 
            string rp(root);
            dpath = root + word[t-1].substr(1);
        }
    }
    while( i < t-1 ){
        des = dpath + "/" + word[i];
        dest = new char[des.length()+1];
        strcpy(dest, des.c_str());
        int d1 = mkdir(dest ,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if( d1 < 0 ){
            cout<<"\nError create_dir command";
            break;
        }
        i++;
    }
}

//-------------------------------- GOTO -----------------------------------------
void gotoPath( vector<string> word ){
    string gpath = word[1];
    char *arr = new char[gpath.length() + 1];
    strcpy(arr , gpath.c_str());
    if ( isDirectory( arr ) ) {
        strcpy(current, arr); 
    }
    else{
        cout<<"\nInavlid Path";
    }
}

//-------------------------------- RENAME FILE -----------------------------------------
void renameFile( vector<string> word ){
    char* f1 = new char[word[1].length()+1];
    strcpy(f1, word[1].c_str());
    char *f2 = new char[word[2].length()+1];
    strcpy(f2, word[2].c_str());
    
    int status = rename(f1,f2);
    if( status != 0 )
    {
        cout<<endl<<"File does not exist"<<endl;
    }
}


//-------------------------------- COPY -----------------------------------------
void copyfile( char *spath, char *dpath ){
    char temp[1024];
	int in, out;
	int fread;
    in = open(spath, O_RDONLY);
	out = open(dpath, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    while( ( fread = read( in, temp, sizeof(temp) ) ) > 0)
		write(out, temp, fread);

    struct stat srcstat, deststat;
	if ( stat(spath, &srcstat) != -1){
    }
    if ( stat(dpath, &deststat) != -1){
    } 
    int status1=chown( dpath, srcstat.st_uid, srcstat.st_gid);
    int status2=chmod(dpath, srcstat.st_mode);
}

void copydirectory( char *spath, char *dpath ){
    int dstatus= mkdir(dpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	 if( dstatus == -1 ){
        return;
	}
    DIR *dr;
	struct dirent *dir;
	dr = opendir( spath );
    if( dr ){
        while ((dir = readdir(dr)) != NULL) {
            if( (string(dir->d_name) == "..") || (string(dir->d_name) == ".") )	{
            }
            else{
                string finalsrcPath= string( spath ) + "/" + string(dir->d_name);
                char* npath = new char[finalsrcPath.length() + 1];
                strcpy(npath, finalsrcPath.c_str());

                string finaldestPath=string( dpath ) + "/" +string(dir->d_name);
                char* ndpath = new char[finaldestPath.length() + 1];
                strcpy(ndpath, finaldestPath.c_str());

                struct stat dstats;
                if (stat( npath, &dstats ) == -1)
                    perror("lstat");
                else{
                    if( (S_ISDIR ( dstats.st_mode ) ) ){
                        copydirectory(npath, ndpath);
                    }
                    else{
                        copyfile(npath, ndpath);
                    }
                }
            }
        }
    }
    else{
        return;
    }
}

void copyCommand( vector<string> word ){
    int t = word.size();
    int i = 0;

    string destPath = word[t-1];
    if( destPath[0] == '~' ){
        destPath = destPath.substr(1);
        string rp(root);
        destPath = rp + destPath;
    }

    for(int i=1 ; i<t-1 ; i++ ){
        
        //get file name from path
        string gPath = word[i];
        int pos = gPath.find_last_of("/\\");
        string fileName = gPath.substr(pos+1, gPath.length());

        char *spath = new char[gPath.length() + 1];
        strcpy(spath , gPath.c_str());
        // spath ----> source path

        //append file name in destination path
        destPath = destPath + "/" + fileName;
        char *dpath = new char[destPath.length() + 1];
        strcpy(dpath , destPath.c_str());
        // dpath ----> destination path

        if(isDirectory(spath)){
			copydirectory( spath, dpath );
		}
		else{
	    	copyfile( spath, dpath );
		}
    }
}



//-------------------------------- SEARCH -----------------------------------------
vector<string> searchvec;

bool match( string tosearch, string dname ){
    if( tosearch == dname )
        return 1;
    else
        return 0;
}

void searchUtil(char *pt, string fname){
    DIR *dr;
	struct dirent *dir;
	dr = opendir(pt);
    if( dr ){
        while( (dir = readdir(dr)) != NULL ){
            if ( (string(dir->d_name) == "..") || (string(dir->d_name) == ".") ){
			}
            else{
                string dname = string(dir->d_name);
				string fpath = string( pt ) + "/" + dname;
				char *npath = new char[fpath.length() + 1];
				strcpy(npath, fpath.c_str());

                struct stat dstats;
                if( stat(npath, &dstats) == -1 ){
                    perror("lstat");
                }
                else{
                    if ((S_ISDIR(dstats.st_mode)))
					{
						if ( match(fname, dname) )
						{
							searchvec.push_back("TRUE");
						}
						searchUtil( npath, fname );
					}
					else
					{
						if ( match(fname, dname) ){
							searchvec.push_back("YES");
						}
					}
                }
            }
        }
    }
    else{
    }
}

void searchCommand( vector<string> word ){
    searchvec.clear();
    string fname = word[1];
    int sp  = sizeof(current) / sizeof(current[0]);
    char *pt = new char[sp];
    strcpy(pt, current);
    searchUtil( pt, fname );
    if( searchvec.size() < 1 ){
        cout<<"\nFALSE";
        return;
    }
    else{
        cout<<"\n";
        for(unsigned int i=0 ; i<searchvec.size() ; i++){
            cout<<"YES  ";
        }
    }
}

//-------------------------------- DELETE FILE -----------------------------------------
void deleteFileUtil( char *pt ){
    //cout<<"\ninside delete = "<<pt; 
    int status = remove( pt );
    if( status!=0 ){
        cout<<"\nError in file deletion";
    }
}

void deleteFile( vector<string> word ){
    unsigned int i = 0;
    for(i=1 ; i<word.size() ; i++ ){
        string gpath = word[i];
        if( word[i][0] == '~' ){
            gpath = word[i].substr(1);
            gpath = root + gpath;
        }
        char *pt = new char[gpath.length()+1];
        strcpy( pt, gpath.c_str() );
        deleteFileUtil( pt );
    }
}

//-------------------------------- DELETE DIRECTORY -----------------------------------------
void deleteDirUtil( char *pt ){
    DIR *d;
    struct dirent *dir;
    d = opendir(pt);
    if( d ){
        while( ( dir = readdir(d) ) != NULL ){
            if( (string(dir->d_name) == "..") || (string(dir->d_name) == ".") )	{
            }
            else{
                string fpath = string( pt ) + "/" + string(dir->d_name);
                char* npath = new char[fpath.length() + 1];
                strcpy(npath, fpath.c_str());

                struct stat dstats;
                if( stat( npath, &dstats ) == -1 ){
                    perror("lstat");
                }
                else{
                    if( (S_ISDIR(dstats.st_mode)) ){
                        deleteDirUtil( npath );
                    }
                    else{
                        deleteFileUtil( npath );
                    }
                }
            }
        } 
        closedir(d);
        int status = rmdir( pt );
        if( status == -1 ){
            cout<<"\nError in directory deletion";
        }
    }
    else{
        cout<<"\nDirectory does not exist";
    }
}

void deleteDir( vector<string> word ){
    unsigned int i = 0;
    for(i=1 ; i<word.size() ; i++ ){
        string gpath = word[i];
        if( word[i][0] == '~' ){
            gpath = word[i].substr(1);
            gpath = root + gpath;
        }
        char *pt = new char[gpath.length()+1];
        strcpy( pt, gpath.c_str() );
        deleteDirUtil( pt );
    }
}


//-------------------------------- MOVE -----------------------------------------
void moveCommand( vector<string> word ){
    int t = word.size();
    int i = 0;

    string destPath = word[t-1];
    if( destPath[0] == '~' ){
        destPath = destPath.substr(1);
        string rp(root);
        destPath = rp + destPath;
    }

    for(int i=1 ; i<t-1 ; i++ ){
        
        //get file name from path
        string gPath = word[i];
        int pos = gPath.find_last_of("/\\");
        string fileName = gPath.substr(pos+1, gPath.length());

        char *spath = new char[gPath.length() + 1];
        strcpy(spath , gPath.c_str());
        // spath ----> source path

        //append file name in destination path
        destPath = destPath + "/" + fileName;
        char *dpath = new char[destPath.length() + 1];
        strcpy(dpath , destPath.c_str());
        // dpath ----> destination path

        if(isDirectory(spath)){
			copydirectory( spath, dpath );
            deleteDirUtil( spath );
		}
		else{
	    	copyfile( spath, dpath );
            deleteFileUtil( spath );
		}
    }
}

void commands( string cmd ){
    //string cmd;
    //getline(cin, cmd);
    vector<string> word;
    int i = 0;
    stringstream ss(cmd);
    string ele;  
    int count = 0;
    while (ss >> ele){
        word.push_back(ele);
    }
    while( 1 ){
        if( word[0] == "create_file" ){
            if( word.size() <= 2 ){
                cout<<"\nLesser arguments for create_file";
                break;
            }
            createFile(word);
            break;
        }
        else if( word[0] == "create_dir" ){
            if( word.size() <= 2 ){
                cout<<"\nLesser arguments for create_dir";
                break;
            }
            createDir(word);
            break;
        }
        else if( word[0] == "search" ){
            if( word.size() != 2 ){
                cout<<"\nIncorrect number of arguments for search command";
                break;
            }
            searchCommand(word);
            break;
        }
        else if( word[0] == "goto" ){
            if( word.size() !=2 ){
                cout<<"\nIncorrect number of arguments for goto command";
                break;
            }
            gotoPath(word);
            break;
        }
        else if( word[0] == "delete_file" ){
            if( word.size() < 2 ){
                cout<<"\nIncorrect number of arguments for delete_file command";
                break;
            }
            deleteFile(word);
            break;
        }
        else if( word[0] == "copy" ){
            if( word.size() < 3 ){
                cout<<"\nLesser arguments for copy";
                break;
            }
            copyCommand(word);
            break;
        }
        else if( word[0] == "move" ){
            if( word.size() < 3 ){
                cout<<"\nLesser arguments for move";
                break;
            }
            moveCommand( word );
            break;
        }
        else if( word[0] == "rename" ){
            if( word.size() !=3 ){
                cout<<"\nIncorrect number of arguments for rename command";
                break;
            }
            renameFile(word);
            break;
        }
        else if( word[0] == "delete_dir" ){
            if( word.size() !=2 ){
                cout<<"\nIncorrect number of arguments for delete_dir command";
                break;
            }
            deleteDir(word);
            break;
        }
        else {
            cout<<"Invalid command"; 
            break;
        }
    }
}

void commandMode(){
    int ch;
    string cmd="";
    while( 1 ){
        ch = getchar();        
        if( ch == 113 )// q
            exit(1);
        else if( ch == 13 ){// enter
            commands(cmd);
            cmd = "";
            cout<<"\n";
        }
        else if( ch == 32 ){// blank_space
            cout<<" ";
            cmd = cmd + " ";
        }
        else if( ch == 127 ){// back space
            if( cmd != "" ){
                cmd.pop_back();
                printf("\b ");
                printf("\033[%dD", 1);
                //printf("%c[2K", 27);
            }
        }
        else if( ch == 27 ){
            //cout<<"esc";
            break;
        }
        else{
            cout<<(char)ch;
            cmd = cmd + (char)(ch);
        }

    }
}

void navigate(){
    int c;
    printf("\033[%d;%dH", 0, 0);

    prevstack.push(root);
    strcpy(current, root);
//b
    while (1) {
        c = kbget();
        if( c == 13 ){// enter key --------------------------------------------
            //editorRefreshScreen();
            //cout<<"open file  =  "<<dirList[cursorTracker-1]<<"   "<<endl;
            //cout<<"enter "<<cursorTracker<<"  ct";
            string rp(current);
            string fullPath;
            fullPath = rp+"/"+dirList[cursorTracker];
            char *fpath = new char[fullPath.length() + 1];
            strcpy(fpath , fullPath.c_str());            
            if (isDirectory(fpath)) {
                cout<<"directory"<<endl;
                strcpy(current, fpath);
                prevstack.push(fullPath);
                upstack.push_back(fullPath);

                k=0;
                l=22;
                cursorTracker = 0;
                printf("\033[H\033[J");
                printf("\033[%d;%dH", 0, 0);
                ListDirectory( fpath );
                printf("\033[%d;%dH", 0, 0);
            }
            else if(isRegularFile(fpath)){
                if (fork() == 0) {
                    //cout<<"regular file  ="<<dirList[cursorTracker]<<endl;
                    execlp("/usr/bin/xdg-open", "xdg-open",fpath , (char *)0);
                    exit(1);
                }
            }
        }
        else if (c == KEY_ESCAPE || c==113) {//up key-------------------------------------------
            break;
        }
        else if( c == KEY_UP ){
            if( cursorTracker>=1 ){
                cursorTracker--;
                cursorup(1);
            }
        } 
        else if( c == KEY_DOWN ){//down key--------------------------------------------
            if( cursorTracker<=21 ){
                cursorTracker++;
                cursordown(1);
            }
        }
        else if (c == KEY_LEFT) {//left key-------------------------------------------------
            if( !forwstack.empty() ){
                string forpath = forwstack.top();
                prevstack.push(forpath);
                upstack.push_back(forpath);
                 //clear screen
                //cout<<cursorTracker<<" "<<dirList[cursorTracker-1];
                forwstack.pop();
                char *fp = new char[forpath.length()+1];
                strcpy(fp, forpath.c_str());
                DIR *dr = opendir(fp);
                if(!dr){
                    //do nothing
                    continue;
                }
                else{
                    strcpy(current, fp);
                }
                k=0;
                l=22;
                cursorTracker = 0;
                printf("\033[H\033[J");
                printf("\033[%d;%dH", 0, 0);
                flag=1;
                ListDirectory( fp );
                printf("\033[%d;%dH", 0, 0);
                
            }
            //cursorforward(1);
        } 
        else if (c == KEY_RIGHT) {// right key---------------------------------
            if(prevstack.size()>=2){
                string prevpath = prevstack.top();
                forwstack.push(prevpath);

                prevstack.pop();
                upstack.pop_back();

                string ppt = prevstack.top();
                char *cp = new char[ppt.length()+1];
                strcpy(cp, ppt.c_str());
                DIR *dr = opendir(cp);
                if(!dr){
                //do Nothing here
                continue;
                }
                else{
                    strcpy(current, cp);
                }
                k=0;
                l=22;
                cursorTracker = 0;
                printf("\033[H\033[J");
                printf("\033[%d;%dH", 0, 0);
                flag=1;
                ListDirectory( cp );
                printf("\033[%d;%dH", 0, 0);
                                    
            }
            //cursorbackward(1);
        }
        else if( c == 104 ){//h home----------------------------------
            while( !prevstack.empty() ){
                prevstack.pop();
            }
            while( !forwstack.empty() ){
                forwstack.pop();
            }
            prevstack.push(root);
            k=0;
            l=22;
            cursorTracker = 0;
            flag=1;
            strcpy(current, root);
            printf("\033[H\033[J");
            printf("\033[%d;%dH", 0, 0);
            ListDirectory( root );
            printf("\033[%d;%dH", 0, 0);
        }
        else if( c == 107 ){//k vertical overflow-----------------------------
            if( cursorTracker<=0 && dirList.size()>23 && k>=1){
                l--;
                k--;
                cursorTracker--;
                printf("\033[H\033[J");
                printf("\033[%d;%dH", 0, 0);
                ListDirectory( current );
                printf("\033[%d;%dH", 0, 0);
            }
        }
        else if( c == 108 ){//l vertical overflow-----------------------------
            if( cursorTracker>=22 && dirList.size()>23 ){
                l++;
                k++;
                cursorTracker++;
                printf("\033[H\033[J");
                printf("\033[%d;%dH", 0, 0);
                ListDirectory( current );
                printf("\033[%d;%dH", 23, 0);
            }
        }
        else if( c == 127 ){// Backspace---------------------------
            upstack.pop_back();
            string prnt = upstack.back();
            char *arr = new char[prnt.length()+1];
            strcpy(arr, prnt.c_str());
            
            while( !prevstack.empty() ){
                cout<<prevstack.top()<<"  ";
                prevstack.pop();
            }
            while( !forwstack.empty() ){
                forwstack.pop();
            }
            k=0;
            l=22;
            cursorTracker = 0;
            strcpy(current, arr);
            printf("\033[H\033[J");
            printf("\033[%d;%dH", 0, 0);
            ListDirectory( arr );
            printf("\033[%d;%dH", 0, 0);

        }
        else if( c == 58 ){// :
            printf("\033[%d;%dH", 28, 0);
            cout<<"Command Mode";
            printf("\033[%d;%dH", 29, 0);
            //cout<<current;
            //disableRawMode();
            commandMode();
            printf("\033[%d;%dH", 0, 0);
            k=0;
            l=22;
            cursorTracker = 0;
            //strcpy(current, arr);
            printf("\033[H\033[J");
            printf("\033[%d;%dH", 0, 0);
            ListDirectory( current );
            printf("\033[%d;%dH", 0, 0);
        }
        else {
            putchar(c);
        }
    }
    printf("\n");
}
//e


void normalMode(){
    enableRawMode();  
    navigate();    
}

int main() {
    editorRefreshScreen();
    strcpy( root, get_current_dir_name() );  
    ListDirectory( root );
    upstack.push_back(root);
    string pt(root);
    present = pt;
    normalMode();
    printf("\033[%d;%dH", 26, 0);

    return 0;
}
