//------------------------------------------------------------------------------
// GB_conform: conform any matrix to its desired sparsity structure
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, the matrix has any one of four sparsity structures: hypersparse,
// sparse, bitmap, or full.  A bitmap or full matrix never has pending work.  A
// sparse or hypersparse matrix may have pending work (zombies, jumbled, and/or
// pending tuples).  The pending work is not finished unless the matrix is
// converted to bitmap or full.  If this method fails, the matrix is cleared
// of all entries.

// OK: BITMAP

#include "GB.h"

#define GB_FREE_ALL GB_phbix_free (A) ;

//------------------------------------------------------------------------------
// GB_hyper_or_bitmap: ensure a matrix is either hypersparse or bitmap
//------------------------------------------------------------------------------

static inline GrB_Info GB_hyper_or_bitmap
(
    bool is_hyper, bool is_sparse, bool is_bitmap, bool is_full,
    GrB_Matrix A, GB_Context Context
)
{
    GrB_Info info ;
    if (is_full || ((is_hyper || is_sparse) &&
        GB_convert_sparse_to_bitmap_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim)))
    {
        // if full or sparse/hypersparse with many entries: to bitmap
        GB_OK (GB_convert_any_to_bitmap (A, Context)) ;
    }
    else if (is_sparse || (is_bitmap &&
        GB_convert_bitmap_to_sparse_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim)))
    { 
        // if sparse or bitmap with few entries: to hypersparse
        GB_OK (GB_convert_any_to_hyper (A, Context)) ;
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_sparse_or_bitmap: ensure a matrix is either sparse or bitmap
//------------------------------------------------------------------------------

static inline GrB_Info GB_sparse_or_bitmap
(
    bool is_hyper, bool is_sparse, bool is_bitmap, bool is_full,
    GrB_Matrix A, GB_Context Context
)
{
    GrB_Info info ;
    if (is_full || ((is_hyper || is_sparse) &&
        GB_convert_sparse_to_bitmap_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim)))
    {
        // if full or sparse/hypersparse with many entries: to bitmap
        GB_OK (GB_convert_any_to_bitmap (A, Context)) ;
    }
    else if (is_hyper || (is_bitmap &&
        GB_convert_bitmap_to_sparse_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim)))
    { 
        // if hypersparse or bitmap with few entries: to sparse
        GB_OK (GB_convert_any_to_sparse (A, Context)) ;
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_hyper_sparse_or_bitmap: ensure a matrix is hypersparse, sparse, or bitmap
//------------------------------------------------------------------------------

static inline GrB_Info GB_hyper_sparse_or_bitmap
(
    bool is_hyper, bool is_sparse, bool is_bitmap, bool is_full,
    GrB_Matrix A, GB_Context Context
)
{
    GrB_Info info ;
    if (is_full || ((is_hyper || is_sparse) &&
        GB_convert_sparse_to_bitmap_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim)))
    {
        // if full or sparse/hypersparse with many entries: to bitmap
        GB_OK (GB_convert_any_to_bitmap (A, Context)) ;
    }
    else if (is_bitmap)
    {
        if (GB_convert_bitmap_to_sparse_test (A->bitmap_switch,
            GB_NNZ (A), A->vlen, A->vdim))
        { 
            // if bitmap with few entries: to sparse
            GB_OK (GB_convert_bitmap_to_sparse (A, Context)) ;
            // conform between sparse and hypersparse
            GB_OK (GB_conform_hyper (A, Context)) ;
        }
    }
    else // is_hyper || is_sparse
    {
        // conform between sparse and hypersparse
        GB_OK (GB_conform_hyper (A, Context)) ;
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_conform
//------------------------------------------------------------------------------

GrB_Info GB_conform     // conform a matrix to its desired sparsity structure
(
    GrB_Matrix A,       // matrix to conform
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A to conform", GB0) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;
    bool is_hyper = GB_IS_HYPERSPARSE (A) ;
    bool is_sparse = GB_IS_SPARSE (A) ;
    bool is_full = GB_IS_FULL (A) ;
    bool is_bitmap = GB_IS_BITMAP (A) ;
    bool is_full_or_dense_with_no_pending_work = is_full || (GB_is_dense (A)
        && !GB_ZOMBIES (A) && !GB_JUMBLED (A) && !GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // select the sparsity structure
    //--------------------------------------------------------------------------

    switch (A->sparsity)
    { 

        //----------------------------------------------------------------------
        // (1) always hypersparse
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE :

            GB_OK (GB_convert_any_to_hyper (A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (2) always sparse
        //----------------------------------------------------------------------

        case GxB_SPARSE :

            GB_OK (GB_convert_any_to_sparse (A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (3) sparse or hypersparse
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_SPARSE :

            if (is_full || is_bitmap)
            { 
                // if full or bitmap: to sparse
                GB_OK (GB_convert_any_to_sparse (A, Context)) ;
            }
            // conform between sparse and hypersparse
            GB_OK (GB_conform_hyper (A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (4) always bitmap
        //----------------------------------------------------------------------

        case GxB_BITMAP :

            GB_OK (GB_convert_any_to_bitmap (A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (5) hypersparse or bitmap
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_BITMAP :

            // ensure the matrix is hypersparse or bitmap
            GB_OK (GB_hyper_or_bitmap (is_hyper, is_sparse, is_bitmap,
                is_full, A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (6) sparse or bitmap
        //----------------------------------------------------------------------

        case GxB_SPARSE + GxB_BITMAP :

            // ensure the matrix is sparse or bitmap
            GB_OK (GB_sparse_or_bitmap (is_hyper, is_sparse, is_bitmap,
                is_full, A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (7) hypersparse, sparse, or bitmap
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_SPARSE + GxB_BITMAP :

            // ensure the matrix is hypersparse, sparse, or bitmap
            GB_OK (GB_hyper_sparse_or_bitmap (is_hyper, is_sparse,
                is_bitmap, is_full, A, Context)) ;
            break ;

        //----------------------------------------------------------------------
        // (8), (12): bitmap or full
        //----------------------------------------------------------------------

        case GxB_FULL :
        case GxB_FULL + GxB_BITMAP :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            { 
                // otherwise: to bitmap
                GB_OK (GB_convert_any_to_bitmap (A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (9) hypersparse or full
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_FULL :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            { 
                // otherwise: to hypersparse
                GB_OK (GB_convert_any_to_hyper (A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (10) sparse or full
        //----------------------------------------------------------------------

        case GxB_SPARSE + GxB_FULL :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            { 
                // otherwise: to sparse
                GB_OK (GB_convert_any_to_sparse (A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (11) hypersparse, sparse, or full
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_SPARSE + GxB_FULL :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else if (is_bitmap)
            { 
                // if bitmap: to sparse
                GB_OK (GB_convert_bitmap_to_sparse (A, Context)) ;
                // conform between sparse and hypersparse
                GB_OK (GB_conform_hyper (A, Context)) ;
            }
            else
            {
                // conform between sparse and hypersparse
                GB_OK (GB_conform_hyper (A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (13) hypersparse, bitmap, or full
        //----------------------------------------------------------------------

        case GxB_HYPERSPARSE + GxB_BITMAP + GxB_FULL :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            { 
                // ensure the matrix is hypersparse or bitmap
                GB_OK (GB_hyper_or_bitmap (is_hyper, is_sparse, is_bitmap,
                    is_full, A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (14) sparse, bitmap, or full
        //----------------------------------------------------------------------

        case GxB_SPARSE + GxB_BITMAP + GxB_FULL :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            { 
                // ensure the matrix is sparse or bitmap
                GB_OK (GB_sparse_or_bitmap (is_hyper, is_sparse, is_bitmap,
                    is_full, A, Context)) ;
            }
            break ;

        //----------------------------------------------------------------------
        // (15) default: hypersparse, sparse, bitmap, or full
        //----------------------------------------------------------------------

        case GxB_AUTO_SPARSITY :
        default :

            if (is_full_or_dense_with_no_pending_work)
            {
                // if full or all entries present: to full
                GB_convert_any_to_full (A) ;
            }
            else
            {
                // ensure the matrix is hypersparse, sparse, or bitmap
                GB_OK (GB_hyper_sparse_or_bitmap (is_hyper, is_sparse,
                    is_bitmap, is_full, A, Context)) ;
            }
            break ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A conformed", GB0) ;
    return (GrB_SUCCESS) ;
}
