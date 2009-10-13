#include "mrilib.h"

static int acorr_demean = 1 ;

#undef  PHI
#define PHI(x) (1.0-qg(x))       /* CDF of N(0,1) */

#undef  PHINV
#define PHINV(x) qginv(1.0-(x))  /* inverse CDF of N(0,1) */

/*----------------------------------------------------------------------------*/
/*! Pearson correlation of x[] and y[] */

static INLINE float acorrfun( int n, float *x, float *y )
{
   float xv=0.0f , yv=0.0f , xy=0.0f , vv,ww ;
   float xm=0.0f , ym=0.0f ;
   register int jj ;

   if( acorr_demean ){
     for( jj=0 ; jj < n ; jj++ ){ xm += x[jj]; ym += y[jj]; }
     xm /= n ; ym /= n ;
   }
   for( jj=0 ; jj < n ; jj++ ){
     vv = x[jj]-xm; ww = y[jj]-ym; xv += vv*vv; yv += ww*ww; xy += vv*ww;
   }

   if( xv <= 0.0f || yv <= 0.0f ) return 0.0f ;
   return atanhf(xy/sqrtf(xv*yv)) ;
}

/*----------------------------------------------------------------------------*/

static INLINE float acorrfun_jack( int n, float *x, float *y, int qq )
{
   float xv=0.0f , yv=0.0f , xy=0.0f , vv,ww ;
   float xm=0.0f , ym=0.0f ;
   register int jj ;

   if( acorr_demean ){
     for( jj=0 ; jj < n ; jj++ ){ if( jj != qq ){ xm += x[jj]; ym += y[jj]; } }
     xm /= (n-1.0f) ; ym /= (n-1.0f) ;
   }
   for( jj=0 ; jj < n ; jj++ ){
     if( jj != qq ){
       vv = x[jj]-xm; ww = y[jj]-ym; xv += vv*vv; yv += ww*ww; xy += vv*ww;
     }
   }

   if( xv <= 0.0f || yv <= 0.0f ) return 0.0f ;
   return atanhf(xy/sqrtf(xv*yv)) ;
}

/*----------------------------------------------------------------------------*/

static float bca_alpha_1 = 0.05f ;
static float bca_phial_1 = 1.64485 ;

static float bca_alpha_2 = 0.1f ;
static float bca_phial_2 = 1.28155f ;

void THD_bstrap_set_bca_alphas( float al1 , float al2 )
{
   if( al1 <  0.01f    ) al1 = 0.01f    ; else if( al1 > 0.1f ) al1 = 0.1f ;
   if( al2 <= 1.2f*al1 ) al2 = 2.0f*al1 ; else if( al2 > 0.2f ) al2 = 0.2f ;
   bca_alpha_1 = al1 ; bca_phial_1 = (float)qginv(al1) ;
   bca_alpha_2 = al2 ; bca_phial_2 = (float)qginv(al2) ;
}

/*----------------------------------------------------------------------------*/
/* Bias-Corrected accelerated (BCa) boostrap estimation of correlation
   and confidence intervals: M Mudelsee, Mathematical Geology 35:651-665.
*//*--------------------------------------------------------------------------*/

float_pair THD_bstrap_corr( int npt, float *xx, float *yy, float tau, int nboot )
{
   float_pair retval = {0.0f,0.0f} ;
   int kb , ii , qq ;
   float rxy , pboot , *rboot , *xb , *yb ; int qboot ;
   float rjbar , ahat , anum,aden , val , z0hat , alpha_b ;
   float rb_al1 , rb_al2 , rb_au1 , rb_au2 , sig1,sig2,sig , med ;

ENTRY("THD_bstrap_corr") ;

   if( npt < 19 || xx == NULL || yy == NULL ) RETURN(retval) ;

   if( nboot < 666 ) nboot = 666 ;

#pragma omp critical (MALLOC)
   { rboot = (float *)malloc(sizeof(float)*(nboot+2*npt)) ; }
   xb = rboot + nboot ;
   yb = xb + npt ;

   rxy = acorrfun( npt , xx , yy ) ;  /* "raw" correlation */

   pboot = (tau <= 0.5f) ? 0.5f : 0.25f/tau ;  /* prob of random jump */
   qboot = (int)(pboot * ((1u << 31)-1u)) ;    /* scaled to int range */

   /** bootstrap work **/

   for( kb=0 ; kb < nboot ; kb++ ){
     qq = (lrand48() >> 3) % npt ;      /* random starting point */
     xb[0] = xx[qq] ; yb[0] = yy[qq] ;
     for( ii=1 ; ii < npt ; ii++ ){
       if( lrand48() < qboot ) qq = (lrand48() >> 3) % npt ; /* random jump */
       else                    qq = (qq+1) % npt ;           /* next point */
       xb[ii] = xx[qq] ; yb[ii] = yy[qq] ;
     }
     rboot[kb] = acorrfun( npt , xb , yb ) ;
   }

   /* find location of rxy in the sorted rboot array,
      then use that to estimate the bias correction parameter z0hat */

   qsort_float( nboot , rboot ) ;
   for( kb=0 ; kb < nboot && rboot[kb] < rxy ; kb++ ) ; /*nada*/
   z0hat = PHINV(kb/(double)nboot) ;  /* kb = number of rboot < rxy */
   if( z0hat < -1.0f ) z0hat = -1.0f ; else if( z0hat > 1.0f ) z0hat = 1.0f ;

   /** jackknife work to get "acceleration" parameter ahat **/

   rjbar = 0.0f ;
   for( kb=0 ; kb < npt ; kb++ ){
     xb[kb] = acorrfun_jack( npt, xx,yy , kb ) ; rjbar += xb[kb] ;
   }
   rjbar /= npt ;
   anum = aden = 0.0f ;
   for( kb=0 ; kb < npt ; kb++ ){
     val = rjbar - xb[kb] ; anum += val*val*val ; aden += val*val ;
   }
   ahat = anum / (6.0f*aden*sqrtf(aden)) ;
   if( ahat < -0.2f ) ahat = -0.2f ; else if( ahat > 0.2f ) ahat = 0.2f ;

   /* compute bootstrap value at alpha_1 level, adjusted */

   val = z0hat + (z0hat + bca_alpha_1) / ( 1.0f - ahat*(z0hat + bca_alpha_1) ) ;
   val = nboot * PHI(val) ; kb  = (int)val ; val = val-kb ;
   rb_al1 = (1.0f-val)*rboot[kb] + val*rboot[kb+1] ;

   /* compute bootstrap value at alpha_2 level, adjusted */

   val = z0hat + (z0hat + bca_alpha_2) / ( 1.0f - ahat*(z0hat + bca_alpha_2) ) ;
   val = nboot * PHI(val) ; kb  = (int)val ; val = val-kb ;
   rb_al2 = (1.0f-val)*rboot[kb] + val*rboot[kb+1] ;

   /* compute bootstrap value at 1-alpha_1 level, adjusted */

   val = z0hat + (z0hat + 1.0f-bca_alpha_1) / ( 1.0f - ahat*(z0hat + 1.0f-bca_alpha_1) ) ;
   val = nboot * PHI(val) ; kb  = (int)val ; val = val-kb ;
   rb_au1 = (1.0f-val)*rboot[kb] + val*rboot[kb+1] ;

   /* compute bootstrap value at 1-alpha_2 level, adjusted */

   val = z0hat + (z0hat + 1.0f-bca_alpha_2) / ( 1.0f - ahat*(z0hat + 1.0f-bca_alpha_2) ) ;
   val = nboot * PHI(val) ; kb  = (int)val ; val = val-kb ;
   rb_au2 = (1.0f-val)*rboot[kb] + val*rboot[kb+1] ;

   /* now estimate sigma both ways, then combine them */

   sig1 = (rb_au1 - rb_al1) / (2.0f*bca_phial_1) ;
   sig2 = (rb_au2 - rb_al2) / (2.0f*bca_phial_2) ;
   sig  = 0.5f * (sig1+sig2) ;

   /* and estimate middle of distribution */

   val = z0hat + (z0hat + 0.5f) / ( 1.0f - ahat*(z0hat + 0.5f) ) ;
   val = nboot * PHI(val) ; kb  = (int)val ; val = val-kb ;
   med = (1.0f-val)*rboot[kb] + val*rboot[kb+1] ;

   retval.a = med ; retval.b = sig ;

#if 0
   /* convert rboot into alpha values */

   for( kb=0 ; kb < nboot ; kb++ ){
     phab = PHINV( (kb+0.5f)/nboot ) - z0hat ;
     val  = phab / ( 1.0f + aa*phab ) - z0hat ;
     if( val < -5.0f ) val = -5.0f ; else if ( val > 5.0f ) val = 5.0f ;
     phab = PHI(val) ;
     printf("%f %f\n",rboot[kb],val) ;
   }
#endif

   /***/

#pragma omp critical (MALLOC)
   { free(rboot) ; }
   RETURN(retval) ;
}
/*----------------------------------------------------------------------------*/

#undef  A
#define A(i,j) asym[(i)+(j)*nsym]
#undef  XPT
#define XPT(q) ( (xtyp<=0) ? xx+(q)*nn : xar[q] )

/*----------------------------------------------------------------------------*/

static void mean_vector( int n , int m , int xtyp , void *xp , float *uvec )
{
   int nn=n , mm=m , jj ; register int ii ;
   register float *xj , fac ; float *xx=NULL , **xar=NULL ;

   if( nn < 1 || mm < 1 || xp == NULL || uvec == NULL ) return ;

   if( xtyp <= 0 ) xx  = (float * )xp ;
   else            xar = (float **)xp ;

   for( ii=0 ; ii < nn ; ii++ ) uvec[ii] = 0.0f ;

   for( jj=0 ; jj < mm ; jj++ ){
     xj = XPT(jj) ;
     for( ii=0 ; ii < nn ; ii++ ) uvec[ii] += xj[ii] ;
   }

   fac = 1.0f / nn ;
   for( ii=0 ; ii < nn ; ii++ ) uvec[ii] *= fac ;
   return ;
}

/*----------------------------------------------------------------------------*/
/*! Compute the principal singular vector of a set of m columns, each
    of length n, stored in array xx[i+j*n] for i=0..n-1, j=0..m-1.
    The singular value is returned, and the vector is stored into uvec[].
    If the return value is not positive, something bad happened.
*//*--------------------------------------------------------------------------*/

static float principal_vector( int n , int m , int xtyp , void *xp , float *uvec )
{
   int nn=n , mm=m , nsym , ii,jj,kk,qq ;
   double *asym , *deval ;
   register double sum , qsum ; register float *xj , *xk ;
   float sval , fsum , *xx=NULL , **xar=NULL , *mvec ;

   nsym = MIN(nn,mm) ;  /* size of the symmetric matrix to create */

   if( nsym < 1 || xp == NULL || uvec == NULL ) return -666.0f ;

#pragma omp critical (MALLOC)
   { asym  = (double *)malloc(sizeof(double)*nsym*nsym) ;  /* symmetric matrix */
     deval = (double *)malloc(sizeof(double)*nsym) ;       /* its eigenvalues */
   }

   if( xtyp <= 0 ) xx  = (float * )xp ;
   else            xar = (float **)xp ;

   /** setup matrix to eigensolve: choose smaller of [X]'[X] and [X][X]' **/
   /**     since [X] is n x m, [X]'[X] is m x m and [X][X]' is n x n     **/

   if( nn > mm ){                       /* more rows than columns:  */
                                        /* so [A] = [X]'[X] = m x m */
     int n1 = nn-1 ;
     for( jj=0 ; jj < mm ; jj++ ){
       xj = XPT(jj) ;
       for( kk=0 ; kk <= jj ; kk++ ){
         sum = 0.0 ; xk = XPT(kk) ;
         for( ii=0 ; ii < n1 ; ii+=2 ) sum += xj[ii]*xk[ii] + xj[ii+1]*xk[ii+1];
         if( ii == n1 ) sum += xj[ii]*xk[ii] ;
         A(jj,kk) = sum ; if( kk < jj ) A(kk,jj) = sum ;
       }
     }

   } else {                             /* more columns than rows:  */
                                        /* so [A] = [X][X]' = n x n */
     float *xt ; int m1=mm-1 ;
#pragma omp critical (MALLOC)
     xt = (float *)malloc(sizeof(float)*nn*mm) ;
     for( jj=0 ; jj < mm ; jj++ ){      /* form [X]' into array xt */
       if( xtyp <= 0 )
         for( ii=0 ; ii < nn ; ii++ ) xt[jj+ii*mm] = xx[ii+jj*nn] ;
       else
         for( ii=0 ; ii < nn ; ii++ ) xt[jj+ii*mm] = xar[jj][ii] ;
     }

     for( jj=0 ; jj < nn ; jj++ ){
       xj = xt + jj*mm ;
       for( kk=0 ; kk <= jj ; kk++ ){
         sum = 0.0 ; xk = xt + kk*mm ;
         for( ii=0 ; ii < m1 ; ii+=2 ) sum += xj[ii]*xk[ii] + xj[ii+1]*xk[ii+1];
         if( ii == m1 ) sum += xj[ii]*xk[ii] ;
         A(jj,kk) = sum ; if( kk < jj ) A(kk,jj) = sum ;
       }
     }

#pragma omp critical (MALLOC)
     free(xt) ;  /* don't need this no more */
   }

   /** compute the first eigenvector corresponding to largest eigenvalue **/
   /** this eigenvector is stored on top of first column of asym         **/

   ii = symeig_irange( nsym, asym, deval, nsym-1, nsym-1, 0 ) ;

   if( ii != 0 || deval[0] < 0.0 ){
#pragma omp critical (MALLOC)
     { free(deval) ; free(asym) ; }
     return -999.0f ;  /* eigensolver failed!? */
   }

   /** Store singular value (sqrt of eigenvalues), **/

   sval = (float)sqrt(deval[0]) ;

   /** SVD is [X] = [U] [S] [V]', where [U] = desired output vectors

       case n <= m: [A] = [X][X]' = [U] [S][S]' [U]'
                    so [A][U] = [U] [S][S]'
                    so eigenvectors of [A] are just [U]

       case n > m:  [A] = [X]'[X] = [V] [S]'[S] [V]'
                    so [A][V] = [V] [S'][S]
                    so eigenvectors of [A] are [V], but we want [U]
                    note that [X][V] = [U] [S]
                    so pre-multiplying each column vector in [V] by matrix [X]
                    will give the corresponding column in [U], but scaled;
                    below, just L2-normalize the column to get output vector **/

	if( nn <= mm ){                    /* copy eigenvector into output directly */
                                      /* (e.g., more vectors than time points) */

     for( ii=0 ; ii < nn ; ii++ ) uvec[ii] = (float)asym[ii] ;

   } else {  /* n > m: transform eigenvector to get left singular vector */
             /* (e.g., more time points than vectors) */

     qsum = 0.0 ;
     for( ii=0 ; ii < nn ; ii++ ){
       sum = 0.0 ;
       if( xtyp <= 0 )
         for( kk=0 ; kk < mm ; kk++ ) sum += xx[ii+kk*nn] * asym[kk] ;
       else
         for( kk=0 ; kk < mm ; kk++ ) sum += xar[kk][ii]  * asym[kk] ;
       uvec[ii] = sum ; qsum += sum*sum ;
     }
     if( qsum > 0.0 ){       /* L2 normalize */
       register float fac ;
       fac = (float)(1.0/sqrt(qsum)) ;
       for( ii=0 ; ii < nn ; ii++ ) uvec[ii] *= fac ;
     }
   }

   /** make it so that uvec dotted into the mean vector is positive **/

#pragma omp critical (MALLOC)
   { mvec = (float *)malloc(sizeof(float),nn) ; }

   mean_vector( nn , mm , xtyp , xp , mvec ) ;

   fsum = 0.0f ;
   for( ii=0 ; ii < nn ; ii++ ) fsum += mvec[ii]*uvec[ii] ;
   if( fsum < 0.0f ){
     for( ii=0 ; ii < nn ; ii++ ) uvec[ii] = -uvec[ii] ;
   }

   /** free at last!!! **/

#pragma omp critical (MALLOC)
   { free(mvec) ; free(deval) ; free(asym) ; }

   return sval ;
}

#undef XPT
#undef A

/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

float_pair THD_bstrap_vectcorr( int nlen , int nboot ,
                                int xnum , float *xx , int ynum , float *yy )
{
   float_pair retval = {0.0f,0.0f} ;

ENTRY("THD_bstrap_vectcorr") ;

   if( nlen < 3 || xnum < 1 || ynum < 1 || xx == NULL || yy == NULL )
     RETURN(retval) ;

   if( xnum == 1 && ynum == 1 ){
     retval = THD_bstrap_corr( nlen , xx , yy , 0.0f , 999 ) ;
     RETURN(retval) ;
   }

   /***/

   RETURN(retval) ;
}

/*----------------------------------------------------------------------------*/

int main( int argc , char *argv[] )
{
   int iarg=1 ;
   MRI_IMAGE *aim , *bim ;
   float *aar , *bar ;

   if( argc < 3 ) ERROR_exit("need 2 file args") ;

   if( AFNI_yesenv("NO_DEMEAN") ) acorr_demean = 0 ;
   SET_RAND_SEED ;

   aim = mri_read_1D(argv[1]) ; if( aim == NULL ) ERROR_exit("Can't read aim") ;
   bim = mri_read_1D(argv[2]) ; if( bim == NULL ) ERROR_exit("Can't read bim") ;
   if( aim->nx != aim->nx ) ERROR_exit("nx not same") ;

   (void)THD_bstrap_corr( aim->nx, MRI_FLOAT_PTR(aim), MRI_FLOAT_PTR(bim), 3.3f,50000 ) ;
   exit(0) ;
}
