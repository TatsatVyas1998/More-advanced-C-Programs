#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <ifaddrs.h> 
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <pthread.h>
#include <errno.h>
struct threadpram{
	int inuse;
	int clientid;
};
struct dirnode{
	char arr[256];
	int m;
	struct dirnode * next; 
};
struct dirnode * root = NULL;
struct dirnode * back = NULL; 

int soc;
struct sockaddr_in address; // for the server address
struct sockaddr_in clientaddress; // for the connected client address and can be used for multiple clients
void * clientacceptor();
pthread_mutex_t * mutexptr;
int mp =0;
int getmutex( char * pr ) {
	struct dirnode * ptr;
	ptr = root; 
	while( ptr != NULL) {
		if( strcmp(ptr->arr, pr) == 0){
		 return ptr->m;
			
		}
		ptr = ptr->next;
	}
	return -1;
	
	
	
} 
int create(char *projectname){ // creates a directory in existing server directory
	char arr[1000] = "./server_repo/"; 
	char xm[100];
	strcpy ( xm,arr);
	strcat ( xm , projectname); 
	DIR * dr = opendir( xm); 
	if( dr!= NULL){
		return -1; 
	} 
	char hidden[100]= "./server_repo/";
	strcat (hidden , "0");
	strcat( hidden , projectname); 
	strcat( arr, projectname) ;
	if( back == NULL ) { // making link list of mutexes 
		root = (struct dirnode *) malloc (sizeof( struct dirnode) );
		strcpy(root->arr , projectname) ;
		root->m =  mp;
		mutexptr  = (pthread_mutex_t * ) malloc( sizeof( pthread_mutex_t));
		pthread_mutex_init(&mutexptr[mp], NULL);
		mp++;
		mutexptr = realloc( mutexptr , mp* sizeof( pthread_mutex_t));
		root->next = NULL;
		back = root;
	} else { 
		back->next = (struct dirnode *) malloc(sizeof(struct  dirnode));
		strcpy(back->next->arr , projectname) ;
		back->next->m =  mp;
		mp++;
		mutexptr = realloc( mutexptr , mp* sizeof( pthread_mutex_t));
		pthread_mutex_init(&mutexptr[mp], NULL);
		
		back->next->next = NULL;
	    back = back->next;
	} 
	char  commitnumber[100];
	struct stat s = {0};

	if (stat(arr, &s) == -1) {
		if( mkdir(arr, 0700) == 0 && mkdir( hidden , 0755)==0 ){
	   
			strcat( arr , "/Manifest");
			FILE * fd = fopen( arr, "w");
			if( fd!=NULL){
				fputs("version : 1", fd);
				fputs("\n", fd);
				fclose(fd);
				strcpy( commitnumber, hidden);
				strcat( commitnumber , "/.cmn");
				FILE * fp = fopen( commitnumber , "w");
				fputs("1" , fp);
				fclose(fp);
			}else{
				printf( "%s\n" , ".Menifest could not be created");
		  
			}
			return 0;
		}
   
	}
 
	return -1;


}


void add( char * file , char * hash, char * project, char * version) {
	char path[100]= "./server_repo/";
	strcat( path , project);
	strcat(path , "/Manifest");
	FILE * fp = fopen( path , "a");
	if( fp!=NULL){
		fputs( file , fp);
		fputs ( " : ", fp);
		fputs(version , fp);
		fputs ( " : ", fp);
		fputs ( hash, fp);
		fputs ( "\n", fp); 
		 fclose (fp);
	}	
}
void removef( char * projectname, char *filename) {
	char arr[1000] = "./";
	strcat( arr , projectname);
	strcat( arr , "/");
	strcat ( arr, filename) ;
	char  arrmanifest[100] = "./";
	strcat( arrmanifest , projectname);
	strcat( arrmanifest , "/Manifest");
	char newfile[100] = "./";
	char cmp[100] = "./";
	strcat(cmp , projectname);
	strcat( cmp , "/");
	strcat( cmp , filename );
	strcat( newfile , projectname);
	strcat( newfile , "/rep");
	DIR * d = opendir( projectname); // implementation of DIR * jsut to check if the project directory exists on the client side
	if(d) {
		int i =0;
		char buff[sizeof(cmp)];
		FILE * fp  = fopen( arrmanifest, "r" );
		fgets(buff, sizeof(buff), fp); // we don't care about the version number
		memset( buff, 0, strlen(buff));
		while(fscanf(fp, "%*s " "%s " , buff)!= EOF) {
		   if ( strcmp(buff, cmp) == 0){
			  fclose(fp);
			   break;
		   } 
		   fscanf(fp, "%*[^\n]");
		   memset(buff, 0 , strlen(buff));
			i++;
		}
		fp  = fopen( arrmanifest, "r" );
		FILE * fs = fopen (newfile , "w");
		int j =0;
		while( fgets(buff, sizeof(buff) , fp)!= NULL){
			if( j != i) {
				fputs(buff, fs);
				memset(buff, 0 , strlen(buff));
			 }
			j++;
		 }
		 fclose(fs);
		 fclose(fp);
		 remove(arrmanifest);
		 rename(newfile , arrmanifest);

	}else{
		printf( "%s\n" , "the project does not exist" ) ;
	}
   closedir(d);

}
void hash(char * filename ,  char * arr1) { // function that creates a hash for each file
	char * c = (char *) malloc(sizeof(char) *100);
	int errnum;
	strcpy(c,filename);
	memmove(c, c+1, strlen(c));
	char x[100]= "./server_repo";
	strcat(x,c);
	FILE * fp = fopen(x, "rb");
	if(  fp == NULL) {
			
	errnum = errno;
	fprintf(stderr, "Value of errno: %d\n", errno);
	perror("Error printed by perror");
	fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
	printf( "%s\n" , "could not create a hash" );
	arr1= NULL;
	return;
	}
	MD5_CTX s;
	char arr[500];
	ssize_t size;
	unsigned char code[MD5_DIGEST_LENGTH];
		MD5_Init(&s);
		 do
	  {
		size=fread(arr, 1, 500, fp);
		MD5_Update(&s, arr, size);
	  }while(size > 0);
		MD5_Final(code, &s);
		char temp [4] ;
		int i ;
		for ( i = 0 ; i <MD5_DIGEST_LENGTH; i++){  // hash is devided into an array this loop basically cobines all the unsigned charactes into one string

			sprintf(temp, "%02x", code[i]);
			strcat( arr1, temp);
			memset( temp, 0 , strlen(temp) );
		}

}
void hashrecompute(DIR * dr , char * arg){
	readdir(dr);
	readdir(dr);
	char manifest[100]= "./server_repo/";
	strcat( manifest , arg);
	char config[100];
	strcpy( config , manifest);
	strcat( config , "/newmanifest");
	FILE * fc = fopen(config , "w");
	char file[100];
	strcpy ( file , manifest);// just so i can copy the path directly 
	strcat( manifest , "/Manifest");
	FILE * fp = fopen( manifest, "r");
	char tempx[100] , tempy[100];
	fscanf(fp,"%s : " "%s", tempx, tempy); // gets version number 
	int j = atoi(tempy);
	j++;
	sprintf(tempy, "%d", j);
	strcat( tempx , " : ");
	strcat(tempx , tempy);
	fputs( tempx , fc);
	fputs( "\n" , fc);
	char * newhash = (char *) malloc(sizeof( char)* 100); // string to get new hash
	char *fn =(char *) malloc(sizeof( char)* 100);
	struct dirent * d;
	while( ((d = readdir(dr)))){ // goes through the entire direcory 
		strcpy( fn , "./");
		strcat(fn , arg);
		strcat( fn , "/");
		if( strcmp((d->d_name),"Manifest") == 0  || strcmp((d->d_name),"newmanifest") == 0 || strcmp((d->d_name),"commit") == 0){ // just continues of the file is manifest file
			memset(fn , 0, strlen(fn));
			continue;
		}
		strcat( fn , d->d_name);
		hash(fn , newhash); // gets the hash again 
		char file1[100];
		char version[100];
		char h[100];
		char flag[4];
		while( fscanf(fp ,"%s :" " %s :" " %s :" " %s", file1, version, h, flag)!=EOF){
			if( strcmp(file1, fn) ==0){
				if(strcmp( h , newhash) != 0){ // if new hash matches the old hash 
					int i = atoi( version);
					i++;
					char v[100];
					sprintf(v, "%d", i);


					fputs(file1 , fc);
					fputs(" : ", fc);
					fputs( v , fc);
					fputs(" : ", fc);
					fputs( newhash, fc);
					fputs("\n", fc);

				}else {
              
					fputs(file1 , fc);
					fputs(" : ", fc);
					fputs( version , fc);
					fputs(" : ", fc);
					fputs( newhash, fc);
					fputs("\n", fc);
             }
             
          }
			  memset(flag , 0, sizeof(flag));
			  memset(file1 , 0, sizeof(file1));
			  memset(version , 0, sizeof(version));
			  memset(h , 0, sizeof(h));
      }
      fflush(fp);
      fclose(fp); 
      fp = fopen( manifest, "r");
      char sx[100];
      fgets(sx, sizeof(sx), fp);
      memset(sx ,0 ,sizeof(sx));
	  memset(fn , 0, 100);
      memset(newhash , 0, 100);
  }
  fclose(fc);
  free(newhash);
  remove( manifest);
  rename( config , manifest);
 // remove(config);
  
}

void lockandupdate(char * projectname , int c){
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char message[100]; 
	char history[100];
	char arr[1026];
	char directoryname[100] = "./server_repo/";
	strcat( directoryname, projectname);
	char dir[100];
	char commitfile[100] ;
	read( c, &commitfile , sizeof(commitfile));
	if( access( commitfile , F_OK ) != -1 ) {
		strcpy( message, "commit found on server" ); 
		write( c , message , sizeof(message)); 
		strcpy( arr,"tar -C ");                                   // file exists
		strcat( arr , "./server_repo/");
		strcat(arr , projectname);
		strcpy ( dir , arr);
		strcat( arr , " -zcvf ");
		char tarnum[100];
		char num[100] = "./server_repo/0";
		strcat( num , projectname);
		strcpy( history , num);
		strcat( history ,"/history");
		FILE * fh = fopen( history , "a");
		strcat( num , "/rollbacknum");
		if( access( num, F_OK ) != -1){
			FILE * fp = fopen(num , "r");
			fgets( tarnum , 100 , fp);
			fclose(fp);
			remove( num);
			fp = fopen(num , "w");
			int i = atoi(tarnum);
			i++; 
			memset( tarnum , 0 , 100);
			sprintf ( tarnum , "%d", i);
			fputs(tarnum , fp); 
			fclose(fp);
	
		}else{
			FILE * fp = fopen(num , "w");
			strcpy(tarnum , "0");
			fputs(tarnum , fp); 
			fclose(fp);
		}
		strcat( arr, tarnum);
		strcat(arr, projectname);
		strcat( arr , ".tar ./");
		system(arr); // creates the project tarfile
		FILE * fcon = fopen( commitfile , "r");
		char flag[100] , file[100] , version[100] , hash[100] ;
		while(fscanf(fcon, "%s :" "%s :" " %s :" " %s", flag , file, version , hash)!= EOF) {
			if( strcmp(flag,"A") ==0) {
				add( file , hash, projectname, version);	
				fputs( "added : " , fh);
				fputs (file , fh);
				fputs("\n" , fh);
			}if(strcmp(flag , "R")==0){
				removef( projectname, file);
				fputs( "removed : " , fh);
				fputs (file , fh);
				fputs("\n" , fh);
			} if( strcmp(flag, "U")==0){
				fputs( "updated : " , fh);
				fputs (file , fh);
				fputs("\n" , fh);
				// goes to history 
			}
		}
		fclose(fcon);
		DIR * dr = opendir(directoryname);
		readdir(dr);
		readdir(dr);
		char temp1[1002];
		strcpy ( temp1 , dir);
		strcat( temp1, "/");
		struct dirent * d;
		while( ((d = readdir(dr)))){
			if( strcmp( d->d_name , "Manifest") == 0){
			continue;
			}
			char temp2[100];
			strcpy(temp2,temp1);
			strcat( temp2, d->d_name);
			remove( temp2);
		}
		memset(temp1, 0 , sizeof(temp1));
		strcpy (temp1,directoryname);
		strcat( temp1, "/");
		strcat( temp1, "new.tar");
		FILE * fn = fopen( temp1, "w");
		char buf[1026] ;
		memset(arr, 0, sizeof(arr));
		int len = read(c, &buf , sizeof(buf));
		while ( strcmp(buf, "Thanos") != 0){
			fwrite(buf,1,len,fn);
			memset(buf, 0, sizeof( buf));
			len = read(c, &buf , sizeof(buf));
		}
		fclose(fn);
		fclose(fh);
		char untar[100] = "tar -xf ";
		strcat( untar, temp1);
		strcat( untar, " -C ./server_repo/");
		strcat( untar, projectname);
		system(untar);
		remove(temp1);
		dr = opendir(directoryname);
		char deletec[100] = "rm -rf ./server_repo/";
		strcat( deletec, projectname);
		strcat( deletec, "/commit");
		system(deletec);
		hashrecompute(dr, projectname);
   
	
	} else {
		printf("%s\n" , "file does not exist");                                            // file doesn't exist
		strcpy( arr , "file");
		write( c , arr, sizeof(arr));
		pthread_mutex_unlock( &mutex);
		return;
	}
	pthread_mutex_unlock( &mutex);

}

 void rollback( char * project , int i, int soc){
	pthread_mutex_t  mutex ;
	int x = getmutex(project);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char message[40];
	char n[20];
	char file[100];
	strcpy( file, "./server_repo/0");
	 
	strcat(file , project);
	strcat(file , "/rollbacknum");
	FILE * fp = fopen( file , "r");
	if( fp == NULL){
		char x[100] = "./server_repo/"; 
		strcat( x, project); 
		DIR * d = opendir( x); 
		if( d!= NULL) {
			char rem[100] = "rm -rf ";
			char sec[100]; 
			strcat( rem, "./server_repo/"); 
			strcpy( sec , rem); 
			strcat( sec , "0");
			strcat( sec , project);
			strcat( rem, project); 
			system(rem); 
			
			system(sec);
			
			system(sec);
			
			system(rem) ;
			strcpy( message, "Thanos" ); 
			write( soc , message , sizeof(message));
			pthread_mutex_unlock( &mutex);
			return;
		}
		strcpy( message, "0" ); 
		write( soc , message , sizeof(message));
		pthread_mutex_unlock( &mutex);
		return;
	 }
	char num[6]; 
	fgets( num , 100 , fp); 
	int s = atoi( num); 
	if( s<i) {
		strcpy( message, "nonvalid version,max version is: " );
		strcat( message, num); 
		write( soc , message , sizeof(message));
		pthread_mutex_unlock( &mutex);
		return; 
	}else{
		while ( s>i) {
			char m[100]= "./";
			sprintf(n,"%d",s);
			strcat( m , n) ;
			strcat ( m,project); 
			strcat( m , ".tar");
			remove(m);
			s--;
		}
		char tarf[100];
		char pr[100] = "./server_repo/";
		strcat( pr, project);
		strcpy(tarf, pr); 
		strcat( pr , "/*");
		char deletec[100] = "rm -r ";
		strcat( deletec, pr);
		system(deletec);
		if( s>= 0){
			sprintf(n,"%d",s);
			char file1[100] = "./";
			strcat( file1, n); 
			strcat( file1, project);
			strcat( file1, ".tar");
			FILE * f1= fopen( file1, "rb");  
			strcat( tarf, "/new.tar");
				
			FILE * f2 = fopen( tarf, "w");
			char buf[1026] = {0};
			int len;

			while((len = fread(buf,1,1026,f1))){
				printf( "%i\n", len);
				//send_int(len, a);
				printf("\nRead : %d",len);
				fwrite(buf,1,len,f2);
				//printf("    Sent : %d",ret);

			}
			fclose(f1); 
			fclose(f2);
			remove ( file1); 
			char untar[100] = "tar -xf ";
			strcat( untar, tarf);
			strcat( untar, " -C ./server_repo/");
			strcat( untar, project);
			system(untar);
			remove(tarf);
			strcpy( message, "Thanos" ); 
			write( soc , message , sizeof(message));
			fclose(fp); 
			remove( file); 
			fp= fopen( file , "w"); 
			fputs( num , fp); 
			fclose( fp); 
		}else{
			char rem[100] ;
			char sec[100] = "rm -r "; 
			strcpy( rem, "./server_repo/"); 
			strcat( sec , rem); 
			strcat( sec , "0");
			strcat( sec , project);
			strcat( rem, project); 
			remove( rem);
			system(sec); 
			remove( sec);
			//system(rem);
			//system(sec);
			//system(sec);
			strcpy( message, "Thanos" ); 
			write( soc , message , sizeof(message));
			pthread_mutex_unlock( &mutex);
			return;
		
		
		}
			
		
		
	}
 
 
 }


void sendmanifest(char *projectname, int a){
    pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char  c[100] = "./server_repo";
	strcat(c , "/");
	strcat ( c, projectname);
	strcat( c , "/Manifest");
	FILE * fp = fopen ( c , "r");
	if( fp== NULL) {
		printf( "%s\n", "no menifest problem have occured");
		return ;
	 }
	char arr[10000];
	while( fgets(arr , sizeof(arr) , fp) !=NULL) {
		write(a , arr , sizeof(arr));
		memset(arr , 0 , sizeof(arr));
	}
	memset(arr , 0 , sizeof(arr));
	strcpy(arr , "Thanos");
	write( a , arr, sizeof(arr) );
	fclose(fp);
	pthread_mutex_unlock( &mutex);
}


void sendproject(char * projectname, int a){ // creates the .tar file in the main directory where the executable is present
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char  c[100] = "tar -C ";
	strcat( c , "./server_repo/");
	strcat(c , projectname);
	strcat( c , " -zcvf ");
	strcat( c , projectname);
	strcat( c , ".tar ./");
	system(c);
	memset( c , 0 , sizeof(c));
	strcpy(c ,projectname);
	strcat( c, ".tar");
	FILE * fp = fopen( c, "rb");
	if( fp == NULL) {
		printf( "%s\n", "file cannot be one check the projectname ");
		return ;
	}
	
	char buf[1026] = {0};
	int len;

	while((len = fread(buf,1,1026,fp))){
		printf( "%i\n", len);
		//send_int(len, a);
		printf("\nRead : %d",len);
        write(a,buf,sizeof(buf));
        //printf("    Sent : %d",ret);




	}
	memset( buf , 0,sizeof(buf));
	//send_int(6, a);
	strcpy( buf, "Thanos");
	write(a,buf,sizeof(buf));
	fclose(fp);
	pthread_mutex_unlock( &mutex);
}
int commitval(char * projectname){
	
	char arr[100] = "./server_repo/0";// replace 0 with "." at the end
	strcat( arr , projectname);
	strcat( arr , "/cmn");
	char getnum[100];
	FILE * fp = fopen ( arr, "r");
	if( fp !=NULL) {
		fscanf(fp, "%s", getnum);
		fclose(fp);
		remove(arr);
	}else{
		strcpy(getnum, "0"); 
	}
	fp= fopen( arr , "w");
	int n = atoi( getnum);
	int m = n+1;
	memset( getnum , 0 , sizeof(getnum));
	sprintf(getnum, "%d", m);
	fputs(getnum , fp);
	fclose( fp);
	
	return n;
	
}
void getcommit(char * projectname, int a){
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	int c = commitval(projectname);
	char arr[100] = "./server_repo/";
	strcat( arr , projectname); 
	strcat( arr , "/");
	 //char d[100]; 
	 //strcpy( d,arr);
	strcat( arr, "commit");
	DIR * dr = opendir( arr);
	if( dr == NULL){
		if(mkdir( arr , 0700) ==0){
			printf( "%s\n" , " sucessfully created commitdirectory");
		}else{
			printf( "%s\n" , " unsucessfully creating commitdirectory");
			return;
		}	
	}
	closedir(dr);
	char num[100];
	sprintf( num , "%d" , c);
	strcat( arr, "/");
	strcat( arr , num);
	strcat( arr , "config");
	FILE * fp = fopen ( arr, "w");
	char readline[10000]; 
	read( a, &readline , sizeof( readline) );
	while( strcmp( readline, "Thanos") != 0){
		fputs( readline, fp);
		memset( readline , 0,strlen(readline));
		read(a , &readline , sizeof(readline));
	}
	
	fclose(fp);
	write(a,arr,sizeof(arr));

	pthread_mutex_unlock( &mutex);
}


void sendupgrade( char * projectname , int a){
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char arr[100]= "./server_repo/";
	strcat( arr , projectname);
	char dirn[100]= "./server_repo/0";
	strcat( dirn, projectname);
	strcat( dirn, "/projectname");
	strcat(dirn , "upgrade"); 
	mkdir( dirn , 0700);
	char ffake[100];
	strcpy (ffake,dirn);
	strcat( ffake , "/fakem");
	FILE * ff= fopen( ffake , "w");
	char manifest[100];
	strcpy( manifest, arr);
	strcat( arr, "/Manifest");
	FILE * fm =  fopen( arr , "r");
	char temp[1000];
	fgets(temp , sizeof(temp), fm);
	fputs(temp , ff);
	 
	memset(temp , 0 , sizeof(temp));
	char filename[100];
	read( a , &filename, sizeof(filename));
	printf( "%s\n", "message recieved");
	if( strcmp( filename , "Thanos") == 0) {
		char deletec[100] = "rm -R ";
		strcat(deletec , dirn);
		system(deletec);
		system(deletec);
		pthread_mutex_unlock( &mutex);
		return;
	} 
	while( strcmp( filename , "Thanos") != 0){
		FILE * add;
		char arrfile[100] , arrversion[100] , arrhash[100];
		while(fscanf(fm,"%s : " "%s : " "%s[^\n]", arrfile, arrversion, arrhash) !=EOF){
			if( strcmp(arrfile , filename) == 0) {
				fputs(arrfile , ff);
			    fputs( " : ", ff);
			    fputs (arrversion , ff);
			    fputs( " : ", ff);
			    fputs (arrhash, ff);
			    fputs("\n", ff);
			    
			     char * addfile = (char *) malloc(sizeof(char) *50);
			     strcpy(addfile , arrfile);
			     memmove(addfile, addfile+1, strlen(addfile)); 
			     memset(temp, 0 , sizeof(temp));
			     strcpy ( temp, "./server_repo");
			     strcat (temp , addfile);
			     char * filename = strtok(arrfile , "/");
			     filename= strtok( NULL , "/");
			     filename = strtok(NULL, "");
			     char newfile[100];
			     strcpy(newfile, dirn);
			     strcat(newfile, "/");
			     strcat(newfile, filename);
			     add = fopen(newfile , "w");
			     FILE * r = fopen(temp , "r");
			     char buff[1000];
			     while( fgets(buff, sizeof(buff) , r) !=NULL){
					 fputs( buff, add);
					 memset(buff, 0, sizeof(buff));
				 }
				 fclose( add);
				 fclose(r);
				 free(addfile);
			    }
			    
			 memset(arrfile, 0 ,sizeof(arrfile));
			 memset(arrversion, 0 ,sizeof(arrversion));
			 memset(arrhash, 0 , sizeof(arrhash));
		}
		fclose(fm); 
		fm = fopen(arr, "r");
		fgets(temp, sizeof(temp), fm);
		memset( filename , 0, sizeof(filename));
		read( a , &filename, sizeof(filename));
	}
	printf( "%s\n", "file recieved");
	fclose(ff);
	char  c[100] = "tar -C ";
	
	strcat(c , dirn);
	strcat( c , " -zcvf ");
	strcat( c , projectname);
	strcat( c , "x.tar ./");
	system(c);
	memset( c , 0 , sizeof(c));
	strcpy(c ,projectname);
	strcat( c, "x.tar");
	FILE * fp = fopen( c, "rb");
	char buf[1026] = {0};
	int len;

	while((len = fread(buf,1,1026,fp))){
		printf( "%i\n", len);
		//send_int(len, a);
		printf("\nRead : %d",len);
        write(a,buf,sizeof(buf));
        //printf("    Sent : %d",ret);




	}
	memset( buf , 0,sizeof(buf));
	//send_int(6, a);
	strcpy( buf, "Thanos");
	write(a,buf,sizeof(buf));
	printf( "%s", "file sent" );
	fclose(fp);
	char xtar[100] = "./";
	strcpy( xtar, projectname);
	strcat (xtar , "x.tar");
	remove(xtar);
	char deletec[100] = "rm -rf ";
	strcat(deletec , dirn);
	system(deletec);
	pthread_mutex_unlock( &mutex);
}
void sendhistory( char * projectname ,int  client){
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char arr[100] = "./server_repo/0";
	strcat( arr, projectname);
	strcat( arr, "/history");
	char buf[100];
	FILE * fp = fopen( arr , "r");
	while( fgets( buf , sizeof( buf) , fp) != NULL) {
		write( client, buf , sizeof(buf));
		memset( buf,0, sizeof(buf));
	}
	strcpy( buf, "Thanos");
	write(client , buf , sizeof(buf));
	fclose(fp);
  	pthread_mutex_unlock( &mutex);
}
void sendcur( char * projectname ,int  client){
	pthread_mutex_t  mutex ;
	int x = getmutex(projectname);
	mutex = mutexptr[x];
	pthread_mutex_lock( &mutex);
	char arr[100] = "./server_repo/";
	strcat( arr, projectname);
	strcat( arr, "/Manifest");
	char buf[100];
	FILE * fp = fopen( arr , "r");
	while( fgets( buf , sizeof( buf) , fp) != NULL) {
		write( client, buf , sizeof(buf));
		memset( buf,0, sizeof(buf));
	}
	strcpy( buf, "Thanos");
	write(client , buf , sizeof(buf));
	fclose(fp);
	pthread_mutex_unlock( &mutex);
}

void * clientwork( void * a) {  // does work on client's requests
	char command[100];
	struct threadpram * p = (struct threadpram *) a;
	int client = p->clientid;
	read( client , command , sizeof(command));
	char projectname[100];
	read( client , projectname, sizeof(projectname));
	char filename[100];
	read( client , filename, sizeof(filename));

	if( strcmp(command, "create") == 0) {
		if (create( projectname)  == 0) {
			send ( client, "success" , 7,0);
		}else {
			send ( client, "FAIL" , 4,0);
		}
	}
	if( strcmp(command , "destroy") == 0){
		rollback(projectname , -1, client);
	}
	if( strcmp( command , "commit") == 0 || strcmp( command , "update") ==0){
		sendmanifest(projectname, client);

	}
	if ( strcmp( command, "commit")==0){
		getcommit( projectname , client);
	}
	if( strcmp ( command, "checkout") ==0 ){
		sendproject( projectname, client);

	}
	if( strcmp( command , "push") == 0){
	
		lockandupdate( projectname , client) ;
	}
	if( strcmp(command , "upgrade") ==0) {
	
		sendupgrade( projectname , client); 
	}
	if( strcmp(command , "currentversion") ==0) {
		sendcur( projectname , client); 
	
	}if( strcmp(command , "history") ==0) {
		sendhistory( projectname , client); 
	
	}if( strcmp( command, "rollback" )== 0){
		int i= atoi(filename); 
		rollback(projectname , i, client);
	}


	p->inuse = 0;
	pthread_exit(0);




	return NULL;
}








void makeserverdir(){ // makes server directory if does not exist

	struct stat s = {0};

	if (stat("./server_repo", &s) == -1) {
		mkdir("./server_repo", 0700);
	}

}
char * getip() {
	char host[256] ;
	char * ip;
	struct hostent * hosten;
	gethostname(host, sizeof(host));
	hosten = gethostbyname(host);
	ip = inet_ntoa(*((struct in_addr*)hosten->h_addr_list[0]));
	//printf("%s\n" , ip);
	return ip;
}
int main( int argc , char ** argv) {
	makeserverdir();

	if ( argc < 2) {

		printf("%s\n" , "please supply the port number");
		return 0;
	}
	char * p = argv[1] ;
	int port = atoi(p);
	soc = socket( AF_INET, SOCK_STREAM, 0);

	if( soc<0 ) {
		printf("%s" , "server socket could not be created");
		return 0;
	}
	if( soc ==0) {
		printf( "%s\n" , "socket could not be created");
		return 0;

	}
	//printf( "%s\n" , getip());

	address.sin_family = AF_INET; // knows the type of the address to work with
	address.sin_port = htons(port);                           // port that we need to conenct
	address.sin_addr.s_addr = inet_addr(getip());
	//printf( "%i\n" , getip());

	bind(soc, (struct sockaddr* ) & address , sizeof(address)); // bind the ip to the socket
	pthread_t mainthread;
	// creating attributes for the mian thread to work with
	pthread_attr_t  mainthreadattr;
	pthread_attr_init(&mainthreadattr);
	pthread_create(&mainthread, &mainthreadattr, clientacceptor, NULL);
	pthread_join(mainthread , NULL);

	close(soc);


	return 0;
}


void  * clientacceptor(){
	listen(soc , 50); // putting 10 as default to listen to 10 clients
	struct threadpram p[50];
	pthread_t newthread[50];
	int i =0;
	while(1) {
		int a;
		socklen_t clientsize = sizeof(clientaddress);
		if( (a = accept(soc ,(struct sockaddr*) &clientaddress , &clientsize )) < 0 ) {

			printf( "%s" , "could not connect to client: ") ;
			printf( "%d\n" , clientaddress.sin_addr.s_addr);
		}else if( i < 50){
			p[i].clientid = a;
			p[i].inuse = 1;

			pthread_create(&newthread[i], NULL, clientwork, &p[i]);
			i++;
		}else{
			int j ;
			for(j =0; j<50 ; j++){
				if( p[j].inuse == 0){
					p[j].clientid = a;
					p[j].inuse = 1;
					pthread_create(&newthread[j], NULL, clientwork, &p[j]);

				}

			}


		}



	}




	return NULL;
}

