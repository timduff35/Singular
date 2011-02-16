/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id$ */

/*
* ABSTRACT - all basic methods to manipulate polynomials
*/

/* includes */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <misc/auxiliary.h>
// #include <polys/options.h>
#include <omalloc/omalloc.h>
#include <reporter/reporter.h>
#include <coeffs/numbers.h>
#include <polys/polys.h>
#include <polys/monomials/ring.h>

#ifdef HAVE_PLURAL
// #include <gring.h>
// #include <sca.h>
#endif

/* 1 for polynomial ring, -1 otherwise */
int     pOrdSgn;
// it is of type int, not BOOLEAN because it is also in ip
/* TRUE if the monomial ordering is not compatible with pFDeg */
BOOLEAN pLexOrder;

/* ----------- global variables, set by procedures from hecke/kstd1 ----- */
/* the highest monomial below pHEdge */
poly      ppNoether = NULL;

#ifdef HAVE_RINGS
/* TRUE iff LT(f) | LT(g) */
BOOLEAN pDivisibleByRingCase(poly f, poly g)
{
  int exponent;
  for(int i = (int)pVariables; i; i--)
  {
    exponent = pGetExp(g, i) - pGetExp(f, i);
    if (exponent < 0) return FALSE;
  }
  return nDivBy(pGetCoeff(g), pGetCoeff(f));
}
#endif

BOOLEAN _p_Test(poly p, ring r, int level);

/*2
*make p homogeneous by multiplying the monomials by powers of x_varnum
*assume: deg(var(varnum))==1
*/
poly pHomogen (poly p, int varnum)
{
  pFDegProc deg;
  if (pLexOrder && (currRing->order[0]==ringorder_lp))
    deg=p_Totaldegree;
  else
    deg=pFDeg;

  poly q=NULL, qn;
  int  o,ii;
  sBucket_pt bp;

  if (p!=NULL)
  {
    if ((varnum < 1) || (varnum > pVariables))
    {
      return NULL;
    }
    o=deg(p,currRing);
    q=pNext(p);
    while (q != NULL)
    {
      ii=deg(q,currRing);
      if (ii>o) o=ii;
      pIter(q);
    }
    q = pCopy(p);
    bp = sBucketCreate(currRing);
    while (q != NULL)
    {
      ii = o-deg(q,currRing);
      if (ii!=0)
      {
        pAddExp(q,varnum, (long)ii);
        pSetm(q);
      }
      qn = pNext(q);
      pNext(q) = NULL;
      sBucket_Add_p(bp, q, 1);
      q = qn;
    }
    sBucketDestroyAdd(bp, &q, &ii);
  }
  return q;
}

/*----------utilities for syzygies--------------*/
poly pTakeOutComp(poly * p, int k)
{
  poly q = *p,qq=NULL,result = NULL;

  if (q==NULL) return NULL;
  BOOLEAN use_setmcomp=rOrd_SetCompRequiresSetm(currRing);
  if (pGetComp(q)==k)
  {
    result = q;
    do
    {
      pSetComp(q,0);
      if (use_setmcomp) pSetmComp(q);
      qq = q;
      pIter(q);
    }
    while ((q!=NULL) && (pGetComp(q)==k));
    *p = q;
    pNext(qq) = NULL;
  }
  if (q==NULL) return result;
  if (pGetComp(q) > k)
  {
    pSubComp(q,1);
    if (use_setmcomp) pSetmComp(q);
  }
  poly pNext_q;
  while ((pNext_q=pNext(q))!=NULL)
  {
    if (pGetComp(pNext_q)==k)
    {
      if (result==NULL)
      {
        result = pNext_q;
        qq = result;
      }
      else
      {
        pNext(qq) = pNext_q;
        pIter(qq);
      }
      pNext(q) = pNext(pNext_q);
      pNext(qq) =NULL;
      pSetComp(qq,0);
      if (use_setmcomp) pSetmComp(qq);
    }
    else
    {
      /*pIter(q);*/ q=pNext_q;
      if (pGetComp(q) > k)
      {
        pSubComp(q,1);
        if (use_setmcomp) pSetmComp(q);
      }
    }
  }
  return result;
}

// Splits *p into two polys: *q which consists of all monoms with
// component == comp and *p of all other monoms *lq == pLength(*q)
void pTakeOutComp(poly *r_p, long comp, poly *r_q, int *lq)
{
  spolyrec pp, qq;
  poly p, q, p_prev;
  int l = 0;

#ifdef HAVE_ASSUME
  int lp = pLength(*r_p);
#endif

  pNext(&pp) = *r_p;
  p = *r_p;
  p_prev = &pp;
  q = &qq;

  while(p != NULL)
  {
    while (pGetComp(p) == comp)
    {
      pNext(q) = p;
      pIter(q);
      pSetComp(p, 0);
      pSetmComp(p);
      pIter(p);
      l++;
      if (p == NULL)
      {
        pNext(p_prev) = NULL;
        goto Finish;
      }
    }
    pNext(p_prev) = p;
    p_prev = p;
    pIter(p);
  }

  Finish:
  pNext(q) = NULL;
  *r_p = pNext(&pp);
  *r_q = pNext(&qq);
  *lq = l;
#ifdef HAVE_ASSUME
  assume(pLength(*r_p) + pLength(*r_q) == lp);
#endif
  pTest(*r_p);
  pTest(*r_q);
}

#if 1
poly pTakeOutComp1(poly * p, int k)
{
  poly q = *p;

  if (q==NULL) return NULL;

  poly qq=NULL,result = NULL;

  if (pGetComp(q)==k)
  {
    result = q; /* *p */
    while ((q!=NULL) && (pGetComp(q)==k))
    {
      pSetComp(q,0);
      pSetmComp(q);
      qq = q;
      pIter(q);
    }
    *p = q;
    pNext(qq) = NULL;
  }
  if (q==NULL) return result;
//  if (pGetComp(q) > k) pGetComp(q)--;
  while (pNext(q)!=NULL)
  {
    if (pGetComp(pNext(q))==k)
    {
      if (result==NULL)
      {
        result = pNext(q);
        qq = result;
      }
      else
      {
        pNext(qq) = pNext(q);
        pIter(qq);
      }
      pNext(q) = pNext(pNext(q));
      pNext(qq) =NULL;
      pSetComp(qq,0);
      pSetmComp(qq);
    }
    else
    {
      pIter(q);
//      if (pGetComp(q) > k) pGetComp(q)--;
    }
  }
  return result;
}
#endif

void pDeleteComp(poly * p,int k)
{
  poly q;

  while ((*p!=NULL) && (pGetComp(*p)==k)) pLmDelete(p);
  if (*p==NULL) return;
  q = *p;
  if (pGetComp(q)>k)
  {
    pSubComp(q,1);
    pSetmComp(q);
  }
  while (pNext(q)!=NULL)
  {
    if (pGetComp(pNext(q))==k)
      pLmDelete(&(pNext(q)));
    else
    {
      pIter(q);
      if (pGetComp(q)>k)
      {
        pSubComp(q,1);
        pSetmComp(q);
      }
    }
  }
}
/*----------end of utilities for syzygies--------------*/

/*2
* divides p1 by its leading coefficient if it is a unit
* (this will always be true over fields; but not over coefficient rings)
*/
void pNorm(poly p1)
{
#ifdef HAVE_RINGS
  if (rField_is_Ring(currRing))
  {
    if (!nIsUnit(pGetCoeff(p1))) return;
  }
#endif
  if (p1!=NULL)
  {
    if (pNext(p1)==NULL)
    {
      pSetCoeff(p1,nInit(1));
      return;
    }
    poly h;
    if (!nIsOne(pGetCoeff(p1)))
    {
      number k, c;
      nNormalize(pGetCoeff(p1));
      k = pGetCoeff(p1);
      c = nInit(1);
      pSetCoeff0(p1,c);
      h = pNext(p1);
      while (h!=NULL)
      {
        c=nDiv(pGetCoeff(h),k);
        // no need to normalize: Z/p, R
        // normalize already in nDiv: Q_a, Z/p_a
        // remains: Q
        if (rField_is_Q() && (!nIsOne(c))) nNormalize(c);
        pSetCoeff(h,c);
        pIter(h);
      }
      nDelete(&k);
    }
    else
    {
      if (nNormalize != nDummy2)
      {
        h = pNext(p1);
        while (h!=NULL)
        {
          nNormalize(pGetCoeff(h));
          pIter(h);
        }
      }
    }
  }
}

/*2
*normalize all coefficients
*/
void p_Normalize(poly p,const ring r)
{
  if (rField_has_simple_inverse(r)) return; /* Z/p, GF(p,n), R, long R/C */
  while (p!=NULL)
  {
#ifdef LDEBUG
    if (currRing==r) {nTest(pGetCoeff(p));}
#endif
    n_Normalize(pGetCoeff(p),r);
    pIter(p);
  }
}

// splits p into polys with Exp(n) == 0 and Exp(n) != 0
// Poly with Exp(n) != 0 is reversed
static void pSplitAndReversePoly(poly p, int n, poly *non_zero, poly *zero)
{
  if (p == NULL)
  {
    *non_zero = NULL;
    *zero = NULL;
    return;
  }
  spolyrec sz;
  poly z, n_z, next;
  z = &sz;
  n_z = NULL;

  while(p != NULL)
  {
    next = pNext(p);
    if (pGetExp(p, n) == 0)
    {
      pNext(z) = p;
      pIter(z);
    }
    else
    {
      pNext(p) = n_z;
      n_z = p;
    }
    p = next;
  }
  pNext(z) = NULL;
  *zero = pNext(&sz);
  *non_zero = n_z;
  return;
}

/*3
* substitute the n-th variable by 1 in p
* destroy p
*/
static poly pSubst1 (poly p,int n)
{
  poly qq=NULL, result = NULL;
  poly zero=NULL, non_zero=NULL;

  // reverse, so that add is likely to be linear
  pSplitAndReversePoly(p, n, &non_zero, &zero);

  while (non_zero != NULL)
  {
    assume(pGetExp(non_zero, n) != 0);
    qq = non_zero;
    pIter(non_zero);
    qq->next = NULL;
    pSetExp(qq,n,0);
    pSetm(qq);
    result = pAdd(result,qq);
  }
  p = pAdd(result, zero);
  pTest(p);
  return p;
}

/*3
* substitute the n-th variable by number e in p
* destroy p
*/
static poly pSubst2 (poly p,int n, number e)
{
  assume( ! nIsZero(e) );
  poly qq,result = NULL;
  number nn, nm;
  poly zero, non_zero;

  // reverse, so that add is likely to be linear
  pSplitAndReversePoly(p, n, &non_zero, &zero);

  while (non_zero != NULL)
  {
    assume(pGetExp(non_zero, n) != 0);
    qq = non_zero;
    pIter(non_zero);
    qq->next = NULL;
    nPower(e, pGetExp(qq, n), &nn);
    nm = nMult(nn, pGetCoeff(qq));
#ifdef HAVE_RINGS
    if (nIsZero(nm))
    {
      pLmFree(&qq);
      nDelete(&nm);
    }
    else
#endif
    {
      pSetCoeff(qq, nm);
      pSetExp(qq, n, 0);
      pSetm(qq);
      result = pAdd(result,qq);
    }
    nDelete(&nn);
  }
  p = pAdd(result, zero);
  pTest(p);
  return p;
}


/* delete monoms whose n-th exponent is different from zero */
poly pSubst0(poly p, int n)
{
  spolyrec res;
  poly h = &res;
  pNext(h) = p;

  while (pNext(h)!=NULL)
  {
    if (pGetExp(pNext(h),n)!=0)
    {
      pLmDelete(&pNext(h));
    }
    else
    {
      pIter(h);
    }
  }
  pTest(pNext(&res));
  return pNext(&res);
}

/*2
* substitute the n-th variable by e in p
* destroy p
*/
poly pSubst(poly p, int n, poly e)
{
  if (e == NULL) return pSubst0(p, n);

  if (pIsConstant(e))
  {
    if (nIsOne(pGetCoeff(e))) return pSubst1(p,n);
    else return pSubst2(p, n, pGetCoeff(e));
  }

#ifdef HAVE_PLURAL
  if (rIsPluralRing(currRing))
  {
    return nc_pSubst(p,n,e);
  }
#endif

  int exponent,i;
  poly h, res, m;
  int *me,*ee;
  number nu,nu1;

  me=(int *)omAlloc((pVariables+1)*sizeof(int));
  ee=(int *)omAlloc((pVariables+1)*sizeof(int));
  if (e!=NULL) pGetExpV(e,ee);
  res=NULL;
  h=p;
  while (h!=NULL)
  {
    if ((e!=NULL) || (pGetExp(h,n)==0))
    {
      m=pHead(h);
      pGetExpV(m,me);
      exponent=me[n];
      me[n]=0;
      for(i=pVariables;i>0;i--)
        me[i]+=exponent*ee[i];
      pSetExpV(m,me);
      if (e!=NULL)
      {
        nPower(pGetCoeff(e),exponent,&nu);
        nu1=nMult(pGetCoeff(m),nu);
        nDelete(&nu);
        pSetCoeff(m,nu1);
      }
      res=pAdd(res,m);
    }
    pLmDelete(&h);
  }
  omFreeSize((ADDRESS)me,(pVariables+1)*sizeof(int));
  omFreeSize((ADDRESS)ee,(pVariables+1)*sizeof(int));
  return res;
}

/* Returns TRUE if
     * LM(p) | LM(lcm)
     * LC(p) | LC(lcm) only if ring
     * Exists i, j:
         * LE(p, i)  != LE(lcm, i)
         * LE(p1, i) != LE(lcm, i)   ==> LCM(p1, p) != lcm
         * LE(p, j)  != LE(lcm, j)
         * LE(p2, j) != LE(lcm, j)   ==> LCM(p2, p) != lcm
*/
BOOLEAN pCompareChain (poly p,poly p1,poly p2,poly lcm)
{
  int k, j;

  if (lcm==NULL) return FALSE;

  for (j=pVariables; j; j--)
    if ( pGetExp(p,j) >  pGetExp(lcm,j)) return FALSE;
  if ( pGetComp(p) !=  pGetComp(lcm)) return FALSE;
  for (j=pVariables; j; j--)
  {
    if (pGetExp(p1,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=pVariables; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
    else if (pGetExp(p2,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=pVariables; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k!=0 ; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
  }
  return FALSE;
}
#ifdef HAVE_RATGRING
BOOLEAN pCompareChainPart (poly p,poly p1,poly p2,poly lcm)
{
  int k, j;

  if (lcm==NULL) return FALSE;

  for (j=currRing->real_var_end; j>=currRing->real_var_start; j--)
    if ( pGetExp(p,j) >  pGetExp(lcm,j)) return FALSE;
  if ( pGetComp(p) !=  pGetComp(lcm)) return FALSE;
  for (j=currRing->real_var_end; j>=currRing->real_var_start; j--)
  {
    if (pGetExp(p1,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=pVariables; k>j; k--)
        for (k=currRing->real_var_end; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k>=currRing->real_var_start; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p2,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
    else if (pGetExp(p2,j)!=pGetExp(lcm,j))
    {
      if (pGetExp(p,j)!=pGetExp(lcm,j))
      {
        for (k=currRing->real_var_end; k>j; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        for (k=j-1; k>=currRing->real_var_start; k--)
        {
          if ((pGetExp(p,k)!=pGetExp(lcm,k))
          && (pGetExp(p1,k)!=pGetExp(lcm,k)))
            return TRUE;
        }
        return FALSE;
      }
    }
  }
  return FALSE;
}
#endif

/*2
* returns the length of a (numbers of monomials)
* respect syzComp
*/
poly pLast(poly a, int &l)
{
  if (a == NULL)
  {
    l = 0;
    return NULL;
  }
  l = 1;
  if (! rIsSyzIndexRing(currRing))
  {
    while (pNext(a)!=NULL)
    {
      pIter(a);
      l++;
    }
  }
  else
  {
    int curr_limit = rGetCurrSyzLimit(currRing);
    poly pp = a;
    while ((a=pNext(a))!=NULL)
    {
      if (pGetComp(a)<=curr_limit/*syzComp*/)
        l++;
      else break;
      pp = a;
    }
    a=pp;
  }
  return a;
}
