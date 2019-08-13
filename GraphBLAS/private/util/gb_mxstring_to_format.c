//------------------------------------------------------------------------------
// gb_mxstring_to_format: get the format from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GxB_Format_Value gb_mxstring_to_format  // GxB_BY_ROW or GxB_BY_COL
(
    const mxArray *mxformat             // MATLAB string, 'by row' or 'by col'
)
{

    GxB_Format_Value format ;
    #define LEN 256
    char format_string [LEN+2] ;
    gb_mxstring_to_string (format_string, LEN, mxformat, "format") ;
    if (MATCH (format_string, "by row"))
    {
        format = GxB_BY_ROW  ;
    }
    else if (MATCH (format_string, "by col"))
    {
        format = GxB_BY_COL  ;
    }
    else
    {
        ERROR ("unknown format") ;
    }
    return (format) ;
}

