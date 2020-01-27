//------------------------------------------------------------------------------
// gbburble: get/set the burble setting for diagnostic output
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin <= 1 && nargout <= 1,
        "usage: b = GrB.burble ; or GrB.burble (b)") ;

    //--------------------------------------------------------------------------
    // set the burble, if requested
    //--------------------------------------------------------------------------

    bool b ;

    if (nargin > 0)
    { 
        // set the burble
        CHECK_ERROR (!gb_mxarray_is_scalar (pargin [0]),
            "input must be a scalar") ;
        b = (bool) mxGetScalar (pargin [0]) ;
        OK (GxB_Global_Option_set (GxB_BURBLE, b)) ;
    }

    //--------------------------------------------------------------------------
    // return the burble
    //--------------------------------------------------------------------------

    OK (GxB_Global_Option_get (GxB_BURBLE, &b)) ;
    pargout [0] = mxCreateDoubleScalar (b) ;
    GB_WRAPUP ;
}
