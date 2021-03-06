#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBL_EPSILON 0x1p-52
#define FUDGE (double)1.01

// c can be made const, b and beta are both const except for index 0
// (but index 0 isn't defined because they use 1-indexing)
// z and eps2 are both mutated
// this function should be total memory-wise (all heap-allocated data has static lifetime)
static void dbisect(double c[], double b[], double beta[], int n, int m1,
                    int m2, double eps1, double *eps2, int *z, double x[])

/**************************************************************************
Purpose:
------------

  Calculates eigenvalues lambda_{m1}, lambda_{m1+1},...,lambda_{m2} of
  a symmetric tridiagonal matrix with diagonal c[0],c[1],...,c[n-1]
  and off-diagonal elements b[1],b[2],...,b[n-1] by the method of
  bisection, using Sturm sequences.
  Input parameters:
------------------------
  c[0]..c[n-1] :
        An n x 1 array giving the diagonal elements of the tridiagonal matrix.
  b[1]..b[n-1] :
        An n x 1 array giving the sub-diagonal elements. b[0] may be
        arbitrary but is replaced by zero in the procedure.
  beta[1]..beta[n-1] :
         An n x 1 array giving the square of the  sub-diagonal elements,
         i.e. beta[i] = b[i]*b[i]. beta[0] may be arbitrary but is replaced by
         zero in the procedure.
  n :
         The order of the matrix.
  m1, m2 :
         The eigenvalues lambda_{m1}, lambda_{m1+1},...,lambda_{m2} are
         calculated (NB! lambda_1 is the smallest eigenvalue).
         m1 <= m2must hold otherwise no eigenvalues are computed.
         returned in x[m1-1],x[m1],...,x[m2-1]

  eps1 :
         a quantity that affects the precision to which eigenvalues are
         computed. The bisection is continued as long as
         x_high - x_low >  2*DBL_EPSILON(|x_low|  + |x_high|) + eps1       (1)
         When (1) is no longer satisfied, (x_high + x_low)/2  gives the
         current eigenvalue lambda_k. Here DBL_EPSILON (constant) is
         the machine accuracy, i.e. the smallest number such that
         (1 + DBL_EPSILON) > 1.
  Output parameters:
------------------------
  eps2 :
        The overall bound  for the error in any eigenvalue.
  z :
        The total number of iterations to find all eigenvalues.
  x :
        The array  x[m1],x[m1+1],...,x[m2] contains the computed eigenvalues.
**********************************************************************/
{
  int i;
  double h, xmin, xmax;
  beta[0] = b[0] = 0.0;

  /* calculate Gerschgorin interval */
  xmin = c[n - 1] - FUDGE * fabs(b[n - 1]);
  xmax = c[n - 1] + FUDGE * fabs(b[n - 1]);
  for (i = n - 2; i >= 0; i--) {
    h = FUDGE * (fabs(b[i]) + fabs(b[i + 1]));
    if (c[i] + h > xmax)
      xmax = c[i] + h;
    if (c[i] - h < xmin)
      xmin = c[i] - h;
  }

  /*  printf("xmax = %lf  xmin = %lf\n",xmax,xmin); */

  /* estimate precision of calculated eigenvalues */
  *eps2 = DBL_EPSILON * (xmin + xmax > 0 ? xmax : -xmin);
  if (eps1 <= 0)
    eps1 = *eps2;
  *eps2 = 0.5 * eps1 + 7 * *eps2;
  {
    int a, k;
    double x1, xu, x0;
    double *wu;

    if ((wu = (double *)calloc(n + 1, sizeof(double))) == NULL) {
      fputs("bisect: Couldn't allocate memory for wu", stderr);
      exit(1);
    }

    /* Start bisection process  */
    x0 = xmax;
    for (i = m2; i >= m1; i--) {
      x[i] = xmax;
      wu[i] = xmin;
    }
    *z = 0;
    /* loop for the k-th eigenvalue */
    for (k = m2; k >= m1; k--) {
      xu = xmin;
      for (i = k; i >= m1; i--) {
        if (xu < wu[i]) {
          xu = wu[i];
          break;
        }
      }
      if (x0 > x[k])
        x0 = x[k];

      x1 = (xu + x0) / 2;
      while (x0 - xu > 2 * DBL_EPSILON * (fabs(xu) + fabs(x0)) + eps1) {
        *z = *z + 1;

        /* Sturms Sequence  */

        a = sturm(n, c, b, beta, x1);

        /* Bisection step */
        if (a < k) {
          if (a < m1)
            xu = wu[m1] = x1;
          else {
            xu = wu[a + 1] = x1;
            if (x[a] > x1)
              x[a] = x1;
          }
        } else {
          x0 = x1;
        }
        x1 = (xu + x0) / 2;
      }
      x[k] = (xu + x0) / 2;
    }
    free(wu);
  }
}