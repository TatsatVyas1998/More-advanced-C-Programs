#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <dirent.h>
#include <time.h>
void removeadd( char * project);
void upgrade( char * project, int soc);
void getcheckout( char * project, int soc);
void letsgetcommitment(char * arg , int soc);
void getupdate( char * project , int soc);
void hashrecompute(DIR * dr , char * arg);
void makenewmanifest(char * arg,int soc);
// when implementing hash you would also update the make file inorder for it to work with md5(the inbuild hashing we used)
void hash(char * filename ,  char * arr1) { // function that creates a hash for each file
	FILE * fp = fopen(filename, "r");
	if(  fp == NULL) {
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


void getversion( char * manifestpath, char * a){
	FILE * fp = fopen( manifestpath, "r");
	fscanf(fp , "%*s :" " %s",a );
	fclose(fp);
}
void add( char * projectname , char * filename) {// add meathod to basically add a file in the directory
	char arr[1000] = "./";
	strcat( arr , projectname);
	strcat( arr , "/");
	strcat ( arr, filename) ;
	char  arrmanifest[100] = "./";
	strcat( arrmanifest , projectname);
	strcat( arrmanifest , "/Manifest");
	DIR * d = opendir( projectname); // implementation of DIR * jsut to check if the project directory exists on the client side
	if(d) {
		char * hashcode = (char * ) malloc(sizeof( char) * 100 ) ;
	   	if( access( arr, F_OK ) == -1){
			printf( "%s\n" , "please add the file to the project" );
			return; 
		}
	   	
	   	hash( arr , hashcode);
	   	if (arr ==NULL){
			return;
		}
	   	FILE * fp = fopen( arrmanifest, "a") ;
	   	if( fp == NULL){
			printf( "%s\n" , "clould not acess the manifest file to add the file into project");
			return ;
		}
		
	   	fputs( arr , fp);
	   	fputs( " : 1" , fp);
	   	fputs( " : " , fp) ;
	   	// version number would go here with colon at the end
	   	fputs( hashcode , fp);
		fputs( " : A" , fp); // server hasn't seen it yet
	   	fputs("\n" , fp);
	   	printf ("%s\n" , " File added" );
	   	fclose(fp);
	}else{
		printf( "%s\n" , "please get the project in order to add file to it");
	}


}

void removefile( char * projectname, char *filename) {
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
	if( access( cmp, F_OK ) == -1){
		printf( "%s\n" , "file does not exist" );
		return;	
	}
	
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
void createproject(char * projectname){
	char arr[100] = "./";
	strcat (arr, projectname);
	struct stat s = {0};

	if (stat(arr, &s) == -1) {
		if( mkdir(arr, 0700) == 0 ) {
			strcat( arr , "/Manifest");
			FILE * fd = fopen( arr, "w");
			if( fd!=NULL){
				fputs("version : 1", fd);
				fputs("\n", fd);
				fclose(fd);
			}else{
				printf( "%s\n" , "client .Menifest could not be created");

			}
		}else{
			printf( "%s\n" , "client's project directory could not be created");

			}

	}
}

char * getport(){ // get's prot from the congigure file
	FILE * fp ;
	char * p = (char*) malloc(sizeof(char) *100);
	fp = fopen("configure", "r");
	fgets( p , 100 , fp) ;
	fgets( p , 100 , fp) ;
	return p;

}
char * getip(){ // get's ip from the file

	FILE * fp ;
	char * p = (char*) malloc(sizeof(char) *100);
	fp = fopen("configure", "r");
	fscanf( fp , "%s" ,p);

	return p;
}
void push ( char * project , int soc) {
	char message[100];
	char arr[100] = "./";
	char dstf[100];
	strcat ( arr, project);
	strcat( arr, "/" );
	strcpy ( dstf,arr);
	strcat( dstf, "temp");
	char arrx[100];
	strcpy(arrx, arr);
	strcat( arr , "cmdnum");
	strcat( arrx , "commit");
	char arrcmd[100] ; 
	strcpy( arrcmd, arr);
	FILE * fp = fopen( arr , "r");
	if( fp == NULL) {
		printf( "%s\n" , "please run commit first");
		return; 
	}
	memset( arr , 0 , sizeof( arr) );
	fgets (arr, sizeof(arr) , fp);
	write(soc ,arr , sizeof(arr));
	
	read( soc , &message , sizeof(message));
	if( strcmp( message , "commit found on server") ==0 ) {
	
	
		DIR * dr = opendir( dstf);
		char rmdir[100] = "rm -rf ";
		strcat( rmdir,dstf);
		if(dr == NULL ) {
			mkdir(dstf , 0700);
		
		}
	fclose(fp);
	
	fp = fopen( arrx, "r");
	char dst1[100]; 
	strcpy ( dst1, dstf);
	char flag[100] , file[100] ;
	while( fscanf(fp ,"%s :" "%s[^\n]", flag , file) !=EOF){
    if( strcmp( flag , "R") == 0  ) {
		memset( flag , 0, sizeof(flag));
		memset(file ,0 , sizeof(file ));
		continue;
	}else if( strcmp(flag, "A")==0 || strcmp(flag, "u")==0){
	strcpy ( dst1, dstf);
	strcat( dst1, "/");
	char file1[100];
	strcpy ( file1, file);
	char * x = strtok(file, "/");
	x =strtok(NULL, "/");
	x =strtok(NULL, "");
	printf(x);
	strcat( dst1,x);
    int fd;
    int dst2;
    int k;
    int err;
    unsigned char buffer[4096];

    // Assume that the program takes two arguments the source path followed
    // by the destination path.

    

    fd = open(file1, O_RDONLY);
    dst2 = open(dst1, O_CREAT | O_WRONLY,0666);

    while (1) {
        err = read(fd, buffer, 4096);
        if (err == -1) {
            printf("Error reading file.\n");
            exit(1);
        }
        k = err;

        if (k == 0) break;

        err = write(dst2, buffer, k);
        if (err == -1) {
            printf("Error writing to file.\n");
            exit(1);
        }
    }

    close(fd);
    
    close(dst2);
     
	memset( dst1 , 0 ,sizeof( dst1));
	
}
}// by the end of this the direrectory should be created and files should be in it. 
//let's tar it and send it
	char tardir[100];
	strcpy( tardir,"tar -C ");                                   // file exists
	strcat( tardir , "./");
	strcat(tardir , project);
	strcat( tardir , "/temp");
	
	strcat( tardir , " -zcvf temp.tar ./");
	system( tardir);//tar file created now let's send it;
	FILE *ftar = fopen("temp.tar", "rb");
	char tarr[1026] = {0}; 
	while (fread(tarr,1,1026,ftar)){
		write( soc, tarr , sizeof(tarr)); 
		
	}
	strcpy ( tarr, "Thanos");
	write( soc, tarr , sizeof(tarr)); 
	remove("temp.tar");//clean up 
	system( rmdir);
	fclose(fp);
	remove(arrx);
	remove(arrcmd);
	removeadd(project);
	}else{
		remove(arr);
		remove( arrx);
		printf( "%s\n" , "commit not found on server" );
		return; 
	}
	
}
void printfcur( char * project , int soc){
	char arr[100]; 
	int i =0;
	read(soc , &arr, sizeof(arr) ); 
	printf( "%s\n" , arr);
	printf(" FILE     VERSION      HASH\n");  
	while ( strcmp(arr, "Thanos") !=0){
		if( i ==1){
			printf( "%s\n" , arr);
		}
		i=1;
		memset(arr, 0 ,sizeof(arr));
		read(soc , &arr, sizeof(arr) ); 
	}
	
	printf( "%s\n" , "END !!");
	
}


int main(int argc , char ** argv){

	if ( argc < 3) {
		printf("%s\n", "Error in the parameters");
		return 0;  
	}


	int soc;
	char * command = argv[1];
	if( strcmp(command , "configure") == 0) { // if configure command supply then create a file

		FILE * fp ;
		fp = fopen("configure", "w");
		if( fp != NULL){
		fputs(argv[2], fp);
		fputs("\n", fp);
		fputs( argv[3] , fp);
		fclose(fp);
	}
	return 0;
	}
	if( strcmp(argv[1], "add") == 0){ // to deal with add command since it only applys to client side
		if( argc < 4) {
			printf( "%s\n" , "add command does not have proper inputs");
		}else{
			add( argv[2] , argv[3]);
		}
		return 0;
	}
	if( strcmp(argv[1], "remove") == 0){

		if( argc < 4) {
		printf( "%s\n" , "remove does not have proper inputs");
		return 0;
		}else{

			removefile( argv[2], argv[3]);
		}
		return 0;
	}


	soc = socket( AF_INET, SOCK_STREAM, 0); // creating the client socket 
	if( soc<0 ) {
		printf("%s\n" , "client socket could not be created");
		return 0;
	}
	if( soc ==0) {
		printf( "%s\n" , "socket could not be created");
		return 0;

	}
	struct sockaddr_in address;
	char * port;
	char  * ip;
	port = getport();
	ip = getip();
	int p = atoi(port);
	address.sin_family = AF_INET; // knows the type of the address to work with
	address.sin_port = htons(p);                           // port that we need to conenct
	address.sin_addr.s_addr = inet_addr(ip);                   // server address  "128.6.13.171"

	int a; // connection status
	a = connect( soc, (struct sockaddr *) &address , sizeof(address));
	if( a !=0) {
	printf("%s\n" , "could not connect");
	return 0;
	} else{

		printf("%s\n" , "connected");
	}
	char arr[100];
	int i =1;
	while( i<argc) {
		strcpy(arr, argv[i]);
		send( soc ,   arr, sizeof(arr) , 0);
		memset (arr, 0 , strlen(arr));
		i++;
	}
	if (argc == 3) {
		send( soc ,   arr, sizeof(arr) , 0);
	}
	if( strcmp(argv[1], "create") ==0){
		createproject(argv[2]);
		recv(soc , &arr , sizeof(arr),0); // just recieving a string
	}
	if( strcmp( argv[1], "commit") ==0){
		letsgetcommitment(argv[2], soc);
	}
	if( strcmp ( argv[1] , "checkout" ) == 0) {
		getcheckout( argv[2] , soc) ;
	}
	if( strcmp ( argv[1] , "update") ==0){
		
		getupdate( argv[2] , soc);
	}
	if( strcmp ( argv[1] , "push")==0){
		push( argv[2], soc);
	}
	if( strcmp( argv[1] , "upgrade") ==0) {
		upgrade( argv[2] , soc);
	}
	if( strcmp( argv[1] , "currentversion") ==0 || strcmp( argv[1] , "history")==0) {
		printfcur( argv[2], soc); 
	}
	if( strcmp( argv[1] , "rollback") ==0) {
		char message[40]; 
		read(soc, &message , 40); 
		if( strcmp( message, "Thanos") == 0 ){
			printf("%s\n", "sucess!!");
		}else if ( strcmp( message, "0") == 0) {
			printf("%s\n", "project does not exist");
		}else{
			printf( "%s\n", message);
		}
	}if( strcmp( argv[1] , "destroy") ==0) {
		char message[40]; 
		read(soc, &message , 40); 
		if( strcmp( message, "Thanos") == 0 ){
			printf("%s\n", "sucess!!");
		}else {
			printf("%s\n", "project does not exist");
		}
	}

	printf( "%s\n" , arr);
	close(soc); // closes the socket
	return 0;
}
void removeadd( char * project){
	char arr[100] = "./";
	char newarr[100];
	strcat ( arr, project);
	strcpy( newarr, arr);
	strcat(newarr, "/xManifest");
	strcat(arr, "/Manifest");
	char buff[100];
	FILE * fp = fopen(arr, "r") ;
	fgets(buff, sizeof(buff) , fp);
	FILE * fn = fopen ( newarr, "w");
	fputs(buff, fn);

	memset(buff, 0, sizeof(buff));
	char file1[100], version1[100], hash0[100];
	while( fscanf(fp, "%s :" " %s :" " %s :" " %*s[^\n]", file1, version1, hash0) != EOF) {
		fputs( file1, fn);
		fputs( " : ", fn);
		fputs ( version1, fn);
		fputs ( " : ", fn);
		fputs ( hash0, fn);
		fputs("\n", fn);
		memset( file1, 0 , strlen( file1));
		memset( version1 , 0 , strlen( version1));
		memset( hash0 , 0 , strlen( hash0));
	}
	fclose(fp);
	fclose(fn);
	remove(arr);
	rename(newarr, arr);
}
void upgrade( char * project, int soc) {
	char update[100] = "./";
	strcat( update, project) ;
	char manifest[100];
	char tempx[100];
	strcpy(tempx, update);
	strcpy(manifest, update);
	strcat( update , "/update");
	if( access( update, F_OK ) != -1){
		FILE * fu = fopen( update , "r");
		char flag[10], file[100];
		while(fscanf( fu, "%s : " "%s" , flag , file) !=EOF ){
			 
			if( strcmp( flag, "R") == 0) {
				removefile(project , file);
				 
			}if( strcmp(flag, "A") == 0) {
				write(soc , file , sizeof(file) );
				printf( "%s\n", "message sent");
			}if( strcmp( flag, "M") == 0){
				 removefile(project, file);
				 write(soc , file , sizeof(file) );
				 printf( "%s\n", "message sent");
			}
			memset( flag, 0, sizeof(flag));
			memset( file, 0 , sizeof(file));
		} 
		write(soc , "Thanos" , 6);
		printf( "%s\n" , "file sent" ); 
		int len,ret;
		char buf[1026] ;
		char path[100]= "./";
		strcat( path , project);
		strcat( path , "/");
		strcat( path , project);
		strcat( path , ".tar");
		FILE* fp;
		fp = fopen(path,"w");
		len = read(soc,&buf,sizeof(buf));
		if( strcmp( buf, "Thanos") == 0 ) {
			fclose(fp); 
			remove(path);
			printf("%s" , "'Up to date");
			return;
		}
		while(strcmp( buf , "Thanos")!= 0){
			printf("\nReceived : %d",len);
			//fputs(buf,fp);
			//printf("%s",buf);
			ret = fwrite(buf,1,len,fp);
			if(ret == -1)
			{
				perror("Error writing to file");
			}
			printf("    Write : %d",ret);
			memset( buf, 0 , sizeof(buf));
			len = read(soc,&buf,sizeof(buf));
		}
		fclose(fp); // by now you should have a project folder made and tar file in it
    // let's untar the file by simply calling the system command 
		printf( "%s\n", "file recieved");
		char untar[100] = "tar -xf ";
		strcat( untar, path);
		strcat( untar, " -C ./");
		strcat( untar, project);
		system(untar);
		remove(path);
		strcat( tempx, "/fakem");
		memset(manifest, 0, sizeof(manifest));
		strcpy(manifest, "./");
		strcat( manifest, project);
		strcat(manifest, "/Manifest");
		fp = fopen(manifest, "r");
		FILE * ff = fopen(tempx, "a");
		char buff[1000];
		fgets(buff, sizeof(buff) , fp);
		memset(buff, 0, sizeof(buff));
		while( fgets(buff, sizeof(buff) , fp) !=NULL){
				fputs(buff, ff);
				memset(buff, 0, sizeof(buff));
		}
		fclose(fp);
		fclose(ff); 
		remove(manifest);
		rename(tempx, manifest);
		printf("%s\n", "sucess");
			remove(update);
		
		}else{
			printf("%s\n" , "please update before upgrading" );
			write(soc , "Thanos" , 6);
			return;
		} 
	
	
}
void getupdate( char * project , int soc) {
	int x=0;
	char dr[100] = "./"; 
	strcat( dr , project);
	DIR * d = opendir(dr);
	//char manifest[100] = "Manifest";
	//hashrecompute(d , project);// computes the hash again
	makenewmanifest(project, soc);// gets server's manifest
	closedir(d);
	char clientm[100];
	strcpy( clientm , dr); 
	strcat ( clientm, "/Manifest");
	char * V1 = (char *) malloc( sizeof(char) * 100);
	char * V2 = (char *) malloc( sizeof(char) * 100);
	char serverm[100];
	strcpy( serverm , dr);
	strcat(serverm, "/servermanifest");
	getversion(clientm , V1);
	getversion (serverm , V2);
	// you can send out a message here 
	FILE * fc = fopen(clientm, "r");
	FILE * fs = fopen( serverm, "r");
	char arr[100];
	char arr1[100];
	fgets(arr, sizeof(arr) , fc);
	fgets(arr1, sizeof (arr1), fs);
	char s[100] = "./";
	strcat( s , project) ;
	 // strcat( s , "/");
	strcat( s , "/update") ;
	FILE * fu = fopen( s, "w");
	char file[100], version[100],file1[100], version1[100], hash0[100], hash1[100], flag[100];
	if( fc == NULL || fs == NULL) {
		 printf( "%s\n" , "cannot open one of the manifest files");
		 return;
	}else{
	  int i =0;
	  while( fscanf(fc, "%s :" " %s :" " %s :" " %s[^\n]", file1, version1, hash0, flag) != EOF) {
	  char * livehash = (char *) malloc(sizeof( char)* 100);
	  hash( file1 , livehash);
	  while(fscanf(fs , "%s :" " %s :" " %s[^\n]", file, version, hash1) != EOF) {
			if( strcmp( file, file1) == 0 ) {
			i =1;
			int cv = atoi(version1);
			int cs = atoi(version);
			if( cv != cs && strcmp(V1,V2) != 0){
				if( strcmp(livehash , hash0) ==0){
					fputs("M : ", fu);
					printf( "%s\n" , "M : ");
					printf( "%s\n" , file);
					fputs( file , fu); // can also add more stuff if needed 
					fputs("\n", fu);
				}else{ // conflict has found exit imidiately
					 x=1;
					 printf("%s" ,"conflict : "); 
					 printf( "%s\n" , file) ; 
				}
			}else if( strcmp( hash1 , livehash) != 0  && strcmp(V1,V2) == 0){
				 fputs("U : ", fu);
				 printf( "%s\n" , "U : ");
				 printf( "%s\n" , file);
				 fputs( file , fu); // can also add more stuff if needed 
				 fputs("\n", fu);
			}
			memset( file, 0 , strlen( file));
			memset( version , 0 , strlen( version));
			memset( hash1, 0 , strlen(hash1));
			}
		
		}if( i ==0){
			if( strcmp(V1, V2)!=0){
					fputs("D : ", fu);
					  printf( "%s\n" , "D : ");
					 printf( "%s\n" , file);
					 fputs( file1 , fu); // can also add more stuff if needed 
					 fputs("\n", fu);
		  }else{
					fputs("U : ", fu);
					  printf( "%s\n" , "U : ");
					 printf( "%s\n" , file);
					 fputs( file1 , fu); // can also add more stuff if needed 
					 fputs("\n", fu);
			  }
	  }else{
			i=0;
	  }
		memset( file1, 0 , strlen( file1));
		memset( version1 , 0 , strlen( version1));
		memset( hash0 , 0 , strlen( hash0));
		fclose(fs);
		fs = fopen(serverm, "r");
		memset( arr , 0 , sizeof(arr));
		fgets( arr,  sizeof(arr), fs);
	
	}
	fclose(fc); 
	fc = fopen( 	clientm , "r"); 
	memset( arr , 0 , sizeof(arr));
    fgets( arr,  sizeof(arr), fc);
	i =0;
	while(fscanf(fs ,"%s :" " %s :" " %s[^\n]", file, version, hash1) != EOF) {
		while( fscanf(fc,"%s :" " %s :" " %s :" " %s[^\n]", file1, version1, hash0, flag) != EOF) {
			if( strcmp(file1, file) ==0 ) {
				i=1;
			}	  
			memset( file1, 0 , strlen( file1));
			memset( version1 , 0 , strlen( version1));
			memset( hash0, 0 , strlen(hash0));
			memset( flag, 0 , strlen(flag));
		
		}
		if( i==0 && strcmp(V1,V2) !=0){ // file in server but not in client
			fputs("A : ", fu);
			fputs( file , fu); // can also add more stuff if needed 
			fputs("\n", fu);
				
		}else{
			i =0;
		}
		memset( file, 0 , strlen( file));
		memset( version , 0 , strlen( version));
		memset( hash1 , 0 , strlen( hash1));
		fclose(fc); 
		fc = fopen( 	clientm , "r");
		memset( arr , 0 , sizeof(arr));
		fgets( arr,  sizeof(arr), fc);
	}
	fclose(fs);
	
}
if( x ==1){
	printf( "%s\n" , "please fix the conflicts and run update again");
	remove(s);
}

fclose(fu);
remove(serverm);
}

void getcheckout( char * project, int soc) {
	int len,ret;
    char buf[1026] ;
    if( mkdir(project, 0700) != 0) {
		printf( "%s\n" , " could not create the project directory" );
		return;
	}
    char path[100]= "./";
    strcat( path , project);
    strcat( path , "/");
    strcat( path , project);
    strcat( path , ".tar");
    FILE* fp;
    fp = fopen(path,"w");
	len = read(soc,&buf,sizeof(buf));
    while(strcmp( buf , "Thanos")!= 0){
        printf("\nReceived : %d",len);
        //fputs(buf,fp);
        //printf("%s",buf);
        ret = fwrite(buf,1,len,fp);
        if(ret == -1)
        {
            perror("Error writing to file");
        }
        printf("    Write : %d",ret);
        memset( buf, 0 , sizeof(buf));
        len = read(soc,&buf,sizeof(buf));
    }
    fclose(fp); // by now you should have a project folder made and tar file in it
    // let's untar the file by simply calling the system command 
    char untar[100] = "tar -xf ";
    strcat( untar, path);
    strcat( untar, " -C ./");
    strcat( untar, project);
    system(untar);
    remove(path);
    
}
	
	

// belowthis is work for commit. workin progress and needs to send server the commit file
void makenewmanifest(char * arg, int soc){ // gets menifest from the server and makes file called servermanifest
	char arr[10000];
	read(soc , &arr , sizeof(arr)); // should be the version number 
	char c[100] = "./";
	strcat(c,arg);
	strcat(c, "/servermanifest");
	FILE * fp = fopen(c, "w");
	while( strcmp( arr, "Thanos") != 0){ // Thanos is the message about the end of the file
		fputs( arr, fp);
		memset( arr , 0,strlen(arr));
		read(soc , &arr , sizeof(arr));
	   }
    fclose(fp);
}
void hashrecompute(DIR * dr , char * arg){
	readdir(dr);
	readdir(dr);
	char manifest[100]= "./";
	strcat( manifest , arg);
	char config[100];
	strcpy( config , manifest);
	strcat( config , "/newmanifest");
	FILE * fc = fopen(config , "w");
	char file[100];
	strcpy ( file , manifest);// just so i can copy the path directly 
	strcat( manifest , "/Manifest");
	char  arr[100];
	FILE * fp = fopen( manifest, "r");
	fgets(arr, sizeof(arr), fp); // gets version number 
	fputs( arr , fc);
	memset(arr , 0, strlen(arr));
	char * newhash = (char *) malloc(sizeof( char)* 100); // string to get new hash
	char *fn =(char *) malloc(sizeof( char)* 100);
	struct dirent * d;
	while( ((d = readdir(dr)))){ // goes through the entire direcory 
		strcpy( fn , file);
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
		int m =0;
		while( fscanf(fp ,"%s :" " %s :" " %s :" " %s", file1, version, h, flag)!=EOF){
			if( strcmp(file1, fn) ==0){
				m=1;
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
					if( strcmp(flag,"A") == 0){
						fputs(" : A", fc);
					}
					fputs("\n", fc);

				 }else {
				  
					   fputs(file1 , fc);
					   fputs(" : ", fc);
					   fputs( version , fc);
					   fputs(" : ", fc);
					   fputs( newhash, fc);
					   if( strcmp(flag,"A") == 0){
							fputs(" : A", fc);
						}
					   fputs("\n", fc);
				}
				 
			}
			memset(flag , 0, strlen(flag));
			memset(file1 , 0, strlen(file1));
			memset(version , 0, strlen(version));
			memset(h , 0, strlen(h));
		}if( m == 0){
			printf ( "%s" , "unadded file detacted: ");
			printf( "%s\n" , fn);
			return;
		}else{
			m =0;
		}
		fflush(fp);
		fclose(fp); 
		fp = fopen( manifest, "r");
		memset(fn , 0, 100);
		memset(newhash , 0, 100);
  }
  fclose(fc);
  free(newhash);
  remove( manifest);
  rename( config , manifest);
  remove(config);
  
}

void letsgetcommitment(char * arg , int soc){

	DIR * dr = opendir(arg);
	if( dr == NULL) {
	  printf( "%s\n" , "directory does not open or exist to work with commit command");
	}
	
    makenewmanifest( arg , soc);	// now we should have server's manifest file
     //hashrecompute(dr , arg);
	char sent[100];
	 
	char c[100] = "./";
	strcat( c , arg);
	char m[100] = "./";
	strcat( m , arg);
	char update[100];
	strcpy( update , m);
	strcpy ( sent , m); 
	strcat( update, "/update");
	char s[100] = "./";
	strcat( s , arg) ;
	strcat( s , "/");
	strcat( s , "/commit") ;
	 
	FILE* fn = fopen( update , "r");
	if( fn !=NULL) {// update exists
		fseek (fn, 0, SEEK_END);
		int size = ftell(fn);

		if ( size == 0) {
			printf("update is empty\n");
			remove( update);
		}else{
			printf("please upgrade first\n");
			return;
		}
	}
	strcat( c , "/Manifest");
	strcat(m , "/servermanifest");
	FILE *fp = fopen( c , "r") ;
	FILE *fs = fopen( m , "r");
    char arr[100];
    char arr1[100];
    fgets(arr, sizeof(arr), fp);
    fgets( arr1 , sizeof(arr1) , fs);
    if (strcmp( arr, arr1) ==1 ){
      printf( "%s\n", "commit failed");
      printf( "%s\n" , "please get the same version the project as server");
      return;
    }
	if( fp == NULL || fs == NULL) {
		 printf( "%s\n" , "cannot open one of the manifest files");
		 return;
	 }else {
	 FILE * fc = fopen( s, "w");
     int i =0;
	 char file[100], version[100],file1[100], version1[100], hash0[100], hash1[100], flag[100];
	 while(fscanf(fs , "%s :" " %s :" " %s[^\n]", file, version, hash1) != EOF) {
		  while( fscanf(fp, "%s :" " %s :" " %s :" " %s[^\n]", file1, version1, hash0,flag) != EOF) { // fp in client's manifest
				if( strcmp( file, file1) == 0 ) {
					  i =1;
					  int cv = atoi(version1);
					  int cs = atoi(version);
					  if( cv < cs){
						 printf( "%s\n", " please update you version of the project");
						 return;
					  }
					  memset( file1, 0 , strlen( file1));
					  memset( version1 , 0 , strlen( version1));
					  memset( hash0, 0 , strlen(hash0));
					  break;
				}
				memset( file1, 0 , strlen( file1));
				memset( version1 , 0 , strlen( version1));
				memset( hash0, 0 , strlen(hash0));
		}
		if( i ==0){
			fputs("R :", fc);
			fputs( file , fc);
			fputs( " : " , fc);
			fputs( version, fc);
			fputs( " : ", fc);
			fputs( hash1, fc);
			fputs("\n", fc);
		}else{
			i=0;
		}

			memset( file, 0 , strlen( file));
			memset( version , 0 , strlen( version));
			memset( hash1 , 0 , strlen( hash1));
			fclose(fp);
			fp = fopen(c, "r");
			memset( arr, 0 ,sizeof( arr));
			fgets(arr, sizeof(arr), fp);
	}
	char * newhash = (char *) malloc( sizeof( char) * 100);
	while( fscanf(fp, "%s :" " %s :" " %s :" " %s[^\n]", file1, version1, hash0,flag) != EOF) { 
		hash( file1 , newhash);
		if( strcmp( flag, "A") == 0) {
			fputs("A :", fc);
			fputs( file1 , fc);
			fputs( " : " , fc);
			fputs( version1, fc);
			fputs( " : ", fc);
			if( strcmp( hash0 , newhash) ==0){ 
				fputs( newhash, fc);
			}else{
				fputs( hash0, fc);
			}
			fputs("\n", fc);
				 
		}else if ( strcmp( hash0, newhash) != 0) {
			fputs("u :", fc);
			fputs( file1 , fc);
			fputs( " : " , fc);
			fputs( version1, fc);
			fputs( " : ", fc);
			fputs( newhash, fc);
			fputs("\n", fc);
		}
		memset( file1, 0 , strlen( file1));
		memset( version1 , 0 , strlen( version1));
		memset( hash0, 0 , strlen(hash0));
		memset( flag, 0 , strlen(flag));
		memset( newhash, 0 , strlen(newhash));
	}
	fclose(fp);
	
	fclose(fc);
	}
	remove( m);
	fclose(fs);
  
	FILE * fc = fopen( s, "r");
	char readline[10000]; 
  
	while(fgets( readline ,sizeof ( readline) , fc) !=NULL){
	  write(soc,readline,sizeof(readline));
	  memset( readline , 0 , sizeof( readline));
	}
	strcpy( readline , "Thanos");
	write(soc,readline,sizeof(readline));
	memset( readline , 0 , sizeof( readline));
	strcat( sent , "/cmdnum");
	remove( sent); // if exists 
	fp = fopen(sent , "w"); 
	read(soc , &readline , sizeof(readline));
	fputs( readline , fp); 
	fclose(fp);  
}
