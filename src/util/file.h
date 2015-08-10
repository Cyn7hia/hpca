/**
 * @file       file.h
 * @author     Remi Lebret
 * @brief	   some file functions
 */

#ifndef FILE_H_
#define FILE_H_

// C++ header
#include <stdio.h>
#include <zlib.h>
#include <string>
#include <stdarg.h>
/**
 *  @defgroup Utility
 *
 *  @brief all classes and functions used along a HPCA analysis.
 *  It defines classes for using files, multithreading.
 *  Constants, functions to handle string of characters.
 *
 *  @{
 *
 * 	@class File
 *
 * 	@brief a @c File object creates, opens and closes a file.
 * 	It can also make some operations on the file.
 */
class File
{
  public:
    /**< file name */
    std::string file_name;
    /**< file stream */
    FILE* os;
    /**< file stream with compression */
    gzFile gzos;
    /**< compression ? */
    bool zip;

    /**
     * 	@brief Constructor
     *
     * 	Create a @c File object.
     *
     * 	@param name the file name
     *  @param compression - boolean compression or not? false by default
     */
    File( std::string const & name
        , bool const compression=false
        )
        : file_name(name)
        , os(0), gzos(0)
        , zip(compression)
    {}

    /**
     * 	@brief Destructor
     *
     * 	Release a @c File.
     */
    ~File() {}

    /**
     * 	@brief Open the file.
     *
     * 	Opening modes:
     * 	- "r": read only
     * 	- "w": write only
     * 	- "rw": read and write
     * 	- "a": append
     *
     * 	@param mode the opening mode. Read only by default.
     */
    void open( std::string mode="r" );

    /**
     * 	@brief Close the file
     */
    void close();

    /**
     *  @brief Say whether file is gzipped or not ?
     *
     *  @return boolean - true if gzipped, false otherwise.
     */
    bool gzip();

    /**
     * 	@brief Skip the header
     */
    void skip_header();

    /**
     * 	@brief Count the number of lines into the file
     *
     * 	@return the number of lines
     */
    int number_of_line();
    
    /**
     * 	@brief Count the number of columns into the file
     *
     *  @param delim char which defines the column delimiter
     *  @param skip_header boolean - true if header file needs to be skipped, false by default.
     *
     * 	@return the number of columns
     */
    int number_of_column(const char delim, bool const header=false);
    
    /**
     * 	@brief Jump the @a n first line of file.
     *
     * 	@param n the number of lines to jump
     */
    void jump_to_line( const int n );

    /**
     * 	@brief Check whether a file is readable.
     *  http://cpp.developpez.com/faq/cpp/?page=fichiers#FICHIERS_existence
     *
     *  @return a boolean - true if the file is readable, false otherwise
     */
    bool is_readable();

    /**
     *  @brief write into file
     *
     *  @param str data to write
     */
    int write( char const * str );

    /**
     *  @brief Return next line in stream
     */
    char * getline();

    /**
     *  @brief Flush a stream
     */
    int flush();

    /**
     *  @brief Accessor
     *
     *  Get outputs file name.
     *
     *  @param filename in/out output name
     *  @param compress - boolean compression or not? false by default
     */
    static std::string const get_file_name( std::string const & filename
                                          , bool const compress=false
                                          );
};

/** @} */

#endif /* FILE_H_ */
