/* ============================================================================
 * Name        : util.c
 * Author      : Remi Lebret
 * Copyright   : Idiap Research Institute
 * Description : some utilities
 * ============================================================================
 */

// C++ header
#include "util.h"
#include "constants.h"

// C headers
extern "C"
{
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>   // For stat().
#ifdef __APPLE__
#include <sys/mount.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h> 
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#else
  #include <sys/types.h>  // For stat().
  #include <sys/sysinfo.h>
#endif
}

// C++ headers
#include <algorithm> // remove(), erase()
#include <stdexcept>
#include <fstream>
#include <sstream>

// Check whether a directory exists
bool is_directory(const char * path)
{
  if ( access( path, 0 ) == 0 )
  {
      struct stat status;
      stat( path, &status );

      if ( status.st_mode & S_IFDIR ) { return true;}
      else { throw std::runtime_error("The path you entered is a file: " + std::string(path) + "\n"); }
  }
  else { throw std::runtime_error("Path doesn't exist: " + std::string(path) + "\n");  }
}

/* check whether cooccurrence.bin exists */
bool is_file(const char * path){
    std::ifstream file( path );
    if(file.fail()){
        std::string error_msg = std::string("Cannot find svd file ")
        + std::string( path )
        + std::string(" !!!\n");
        throw std::runtime_error(error_msg);
    }else return true;
}

// get total memory
const unsigned long long int get_total_memory()
{
#ifdef __APPLE__
    struct statfs stats;
    if (statfs("/", &stats))
    {  perror("sysinfo()"); exit(1); }
    
    return (uint64_t)stats.f_bsize * stats.f_bfree;
#elif __linux
    struct sysinfo the_info;
    if(sysinfo(&the_info))
    {  perror("sysinfo()"); exit(1); }
    
    return the_info.totalram;
#else
    // 2GB -1
    return 2147483647;
#endif
}

// get total swap
const unsigned long long int get_total_swap()
{
#ifdef __linux
    struct sysinfo the_info;
    if(sysinfo(&the_info))
    {  perror("sysinfo()"); exit(1); }

    return the_info.totalswap;
#else
    // 0
    return 0;
#endif
}

#ifdef __APPLE__
// get the memory available
unsigned long long int get_available_memory()
{
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics_data_t vm_stats;
    
    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO, 
                                        (host_info_t)&vm_stats, &count))
    {
        return (int64_t)vm_stats.free_count * (int64_t)page_size; 
    }
    else { throw std::runtime_error("Can't get available memory\n");  }
}
#elif __linux
// get the memory available
unsigned long long int get_available_memory()
{
    std::string  data_line;
    size_t found;
    long int availmem=0;
    
    // 'file' stat seems to give the most reliable results
    //
    std::ifstream mem_stream("/proc/meminfo",std::ios_base::in);
    
    if(mem_stream.fail())throw std::runtime_error("cannot open file /proc/meminfo \n");
    
    // Parameter page research
    while (getline(mem_stream, data_line))
    {
        // remove space characters from parameter
        if ( data_line.find("MemFree")!=std::string::npos )
        {
            std::istringstream tokenizer(data_line);
            std::string token;
            
            getline(tokenizer, token, ':');
            getline(tokenizer, token, 'k');
            std::istringstream float_iss(token);
            long int freemem;
            float_iss >> freemem;
            availmem +=  freemem;
            break;
        }
    }
    // check whether the keyword is found
    if (mem_stream.eof()) throw std::runtime_error("MemFree not found in file /proc/meminfo \n");
    
    // Parameter page research
    while (getline(mem_stream, data_line))
    {
        // remove space characters from parameter
        if ( data_line.find("Cached")!=std::string::npos )
        {
            std::istringstream tokenizer(data_line);
            std::string token;
            
            getline(tokenizer, token, ':');
            getline(tokenizer, token, 'k');
            std::istringstream float_iss(token);
            long int cached;
            float_iss >> cached;
            availmem += cached;
            break;
        }
    }
    // check whether the keyword is found
    if (mem_stream.eof()) throw std::runtime_error("Cached not found in file /proc/meminfo \n");
    
    // add swap space if required in constants.h
#ifdef USE_SWAP
    // Parameter page research
    while (getline(mem_stream, data_line))
    {
        // remove space characters from parameter
        if ( data_line.find("SwapFree")!=std::string::npos )
        {
            std::istringstream tokenizer(data_line);
            std::string token;
            
            getline(tokenizer, token, ':');
            getline(tokenizer, token, 'k');
            std::istringstream float_iss(token);
            long int cached;
            float_iss >> cached;
            availmem += cached;
            break;
        }
    }
    // check whether the keyword is found
    if (mem_stream.eof()) throw std::runtime_error("SwapFree not found in file /proc/meminfo \n");
#endif
    
    // file closing
    mem_stream.close();
    
    return availmem*1024;
}
#else
// get the memory available
unsigned long long int get_available_memory()
{
    // 2GB - 1
    return 2147483647;
}
#endif

// get a line in a file
char * get_next_line (FILE * stream)
{
   char *line = NULL;
   if (stream != NULL)
   {
      size_t size = 2;
      line = (char*)malloc (sizeof *line * size);
      if (line != NULL)
      {
         if (fgets (line, size, stream) != NULL)
         {
            int end = 0;
            char *p;
            while (!end && (p = strchr (line, '\n')) == NULL)
            {
               char *tmp = (char*)realloc (line, size * 2);
               if (tmp != NULL)
               {
                  end = fgets (tmp + size - 1, size + 1, stream) == NULL;
                  size *= 2;
                  line = tmp;
               }
               else
               {
                  free (line), line = NULL;
                  end = 1;
               }
            }
            if (!end)
            {
               *p = 0;
            }
         }
         else
         {
            free (line), line = NULL;
         }
      }
   }
   return line;
}

// get a line in a gzip file
char * get_next_gzline ( gzFile stream )
{
   char *line = NULL;
   if (stream != NULL)
   {
      size_t size = 2;
      line = (char*)malloc (sizeof *line * size);
      if (line != NULL)
      {
         if (gzgets (stream, line, size) != NULL)
         {
            int end = 0;
            char *p;
            while (!end && (p = strchr (line, '\n')) == NULL)
            {
               char *tmp = (char*)realloc (line, size * 2);
               if (tmp != NULL)
               {
                  end = gzgets ( stream, tmp + size - 1, size + 1 ) == NULL;
                  size *= 2;
                  line = tmp;
               }
               else
               {
                  free (line), line = NULL;
                  end = 1;
               }
            }
            if (!end)
            {
               *p = 0;
            }
         }
         else
         {
            free (line), line = NULL;
         }
      }
   }
   return line;
}


/* Lowercase a given line */
int lowercase(char *p) {
    for ( ; *p; ++p) *p = tolower(*p);
    return 0;
}

/* check whether the character is a digit */
int check_digit(char a)
{
    int ascii = (int)a;
    if ((ascii>=48)  && (ascii<58))
        return 1;
    
    return 0;
}


/* replace all digits with the special character '0' */
int replace_digit(char* string) {
    char  *temp, *pointer, ch, *start;
    
    temp = string;
    pointer = (char*)malloc(MAX_STRING_LENGTH);
    
    if( pointer == NULL )
    {
        printf("Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }
    
    start = pointer;
    
    while(*temp)
    {
        ch = *temp;
        
        if (check_digit(ch))
        {
            *pointer = '0';
            pointer++;
            temp++;
            ch = *temp;
            while (check_digit(ch) || (ch==',') || (ch=='.')){
                temp++;
                ch = *temp;
            }
            
        }else{
            *pointer = ch;
            pointer++;
            temp++;
        }
    }
    *pointer = '\0';
    
    pointer = start;
    strcpy(string, pointer); /* If you wish to convert original string */
    free(pointer);
    
    return 0;
}

// copy a string of a file in a string
char * string_copy(char *dest, const char *src, int *i, char const delim)
{
   const char *p;
   char *q;

   for(p = src, q = dest; *p != delim && *p != '\0'; p++)
   {
     if( (*p != ' ') && (*p != '"') )
     {  *q = *p; q++;    }
     ++(*i);
   }
   *q = '\0';

   return dest;
}

// remove all occurrences of character C
void remove_all_characters( std::string & Str, char C )
{
    Str.erase(
        std::remove( Str.begin(), Str.end(), C ),
        Str.end() );
}

// remove all characters C at the beginning of Str
std::string remove_first_characters( const std::string & Str, char C )
{
   return Str.substr(
       Str.find_first_not_of( C ) );
}

// find arguments from options in command line
int find_arg(char *str, int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        if(!strcmp(str, argv[i])) {
            if (i == argc - 1) {
                printf("No argument given for %s\n", str);
                exit(1);
            }
            return i;
        }
    }
    return -1;
}

char *strdup (const char *s) {
    char *d = (char*)malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}



