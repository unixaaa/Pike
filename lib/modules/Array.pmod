#define error(X) throw( ({ (X), backtrace()[0..sizeof(backtrace())-2] }) )

constant diff = __builtin.diff;
constant diff_longest_sequence = __builtin.diff_longest_sequence;
constant diff_compare_table = __builtin.diff_compare_table;
constant longest_ordered_sequence = __builtin.longest_ordered_sequence;
constant interleave_array = __builtin.interleave_array;

constant sort = __builtin.sort;
constant everynth = __builtin.everynth;
constant splice = __builtin.splice;
constant transpose = __builtin.transpose;

#if 1
constant filter=predef::filter;
constant map=predef::map;
#else
mixed map(mixed arr, mixed fun, mixed ... args)
{
  int e,s;
  mixed *ret;

  if(mappingp(arr))
    return mkmapping(indices(arr),map(values(arr),fun,@args));

  if(multisetp(arr))
    return mkmultiset(map(indices(arr,fun,@args)));

  if(!(arrayp(arr) || objectp(arr)))
    error("Bad argument 1 to Array.map().\n");

  switch(sprintf("%t",fun))
  {
  case "int":
    if(objectp(arr)) {
      ret=allocate(s=sizeof(arr));
      for(e=0;e<s;e++)
	ret[e]=arr[e](@args);
      return ret;
    }
    else return arr(@args);

  case "string":
    if(objectp(arr)) {
      ret=allocate(s=sizeof(arr));
      for(e=0;e<s;e++)
	ret[e]=arr[e][fun](@args);
      return ret;
    }
    else return column(arr, fun)(@args);

  case "function":
  case "program":
  case "object":
    ret=allocate(s=sizeof(arr));
    for(e=0;e<s;e++)
      ret[e]=fun(arr[e],@args);
    return ret;

  case "multiset":
    return rows(fun, arr);

  default:
    error("Bad argument 2 to Array.map().\n");
  }
}


mixed filter(mixed arr, mixed fun, mixed ... args)
{
  int e;
  mixed *ret;

  if(mappingp(arr))
  {
    mixed *i, *v, r;
    i=indices(arr);
    ret=map(v=values(arr),fun,@args);
    r=([]);
    for(e=0;e<sizeof(ret);e++) if(ret[e]) r[i[e]]=v[e];

    return r;
  }
  if(multisetp(arr))
  {
    return mkmultiset(filter(indices(arr,fun,@args)));
  }
  else
  {
    int d;
    ret=map(arr,fun,@args);
    for(e=0;e<sizeof(arr);e++) if(ret[e]) ret[d++]=arr[e];
    
    return ret[..d-1];
  }
}
#endif

mixed reduce(function fun, array arr, mixed|void zero)
{
  if(sizeof(arr))
    zero = arr[0];
  for(int i=1; i<sizeof(arr); i++)
    zero = fun(zero, arr[i]);
  return zero;
}

mixed rreduce(function fun, array arr, mixed|void zero)
{
  if(sizeof(arr))
    zero = arr[-1];
  for(int i=sizeof(arr)-2; i>=0; --i)
    zero = fun(arr[i], zero);
  return zero;
}

array shuffle(array arr)
{
  int i = sizeof(arr);

  while(i) {
    int j = random(i--);
    if (j != i) {
      mixed tmp = arr[i];
      arr[i] = arr[j];
      arr[j] = tmp;
    }
  }
  return(arr);
}

array permute(array a,int n)
{
   int q=sizeof(a);
   int i;
   a=a[..]; // copy
   
   while (n && q)
   {
      int x=n%q; 
      n/=q; 
      q--; 
      if (x) [a[i],a[i+x]]=({ a[i+x],a[i] });
      i++;
   }  
   
   return a;
}

int search_array(array arr, mixed fun, mixed ... args)
{
  int e;

  if(stringp(fun))
  {
    for(e=0;e<sizeof(arr);e++)
      if(([array(object)]arr)[e][fun](@args))
	return e;
    return -1;
  }
  else if(functionp(fun))
  {
    for(e=0;e<sizeof(arr);e++)
      if(([function]fun)(arr[e],@args))
	return e;
    return -1;
  }
  else if(intp(fun))
  {
    for(e=0;e<sizeof(arr);e++)
      if(([array(function)]arr)[e](@args))
	return e;
    return -1;
  }
  else
  {
    error("Bad argument 2 to filter().\n");
  }
}

array sum_arrays(function foo, array(mixed) ... args)
{
  array ret;
  int e,d;
  ret=allocate(sizeof(args[0]));
  for(e=0;e<sizeof(args[0]);e++)
    ret[e]=foo(@ column(args, e));
  return ret;
}

array sort_array(array foo,function|void cmp, mixed ... args)
{
  array bar,tmp;
  int len,start;
  int length;
  int foop, fooend, barp, barend;

  if(!cmp || cmp==`>)
  {
    foo+=({});
    sort(foo);
    return foo;
  }

  if(cmp == `<)
  {
    foo+=({});
    sort(foo);
    return reverse(foo);
  }

  length=sizeof(foo);

  foo+=({});
  bar=allocate(length);

  for(len=1;len<length;len*=2)
  {
    start=0;
    while(start+len < length)
    {
      foop=start;
      barp=start+len;
      fooend=barp;
      barend=barp+len;
      if(barend > length) barend=length;
      
      while(1)
      {
	if(cmp(foo[foop],foo[barp],@args) <= 0)
	{
	  bar[start++]=foo[foop++];
	  if(foop == fooend)
	  {
	    while(barp < barend) bar[start++]=foo[barp++];
	    break;
	  }
	}else{
	  bar[start++]=foo[barp++];
	  if(barp == barend)
	  {
	    while(foop < fooend) bar[start++]=foo[foop++];
	    break;
	  }
	}
      }
    }
    while(start < length) bar[start]=foo[start++];

    tmp=foo;
    foo=bar;
    bar=tmp;
  }

  return foo;
}

array uniq(array a)
{
  return indices(mkmapping(a,a));
}

array columns(array x, array ind)
{
  array ret=allocate(sizeof(ind));
  for(int e=0;e<sizeof(ind);e++) ret[e]=column(x,ind[e]);
  return ret;
}

array transpose_old(array x)
{
   if (!sizeof(x)) return x;
   array ret=allocate(sizeof(x[0]));
   for(int e=0;e<sizeof(x[0]);e++) ret[e]=column(x,e);
   return ret;
}

// diff3, complement to diff

array(array(array)) diff3 (array a, array b, array c)
{
  // This does not necessarily produce the optimal sequence between
  // all three arrays. A diff_longest_sequence() that takes any number
  // of arrays would be nice.
  array(int) seq_ab = diff_longest_sequence (a, b);
  array(int) seq_bc = diff_longest_sequence (b, c);
  array(int) seq_ca = diff_longest_sequence (c, a);

  array(int) aeq = allocate (sizeof (a) + 1);
  array(int) beq = allocate (sizeof (b) + 1);
  array(int) ceq = allocate (sizeof (c) + 1);
  aeq[sizeof (a)] = beq[sizeof (b)] = ceq[sizeof (c)] = 7;

  for (int i = 0, j = 0; j < sizeof (seq_ab); i++)
    if (a[i] == b[seq_ab[j]]) aeq[i] |= 2, beq[seq_ab[j]] |= 1, j++;
  for (int i = 0, j = 0; j < sizeof (seq_bc); i++)
    if (b[i] == c[seq_bc[j]]) beq[i] |= 2, ceq[seq_bc[j]] |= 1, j++;
  for (int i = 0, j = 0; j < sizeof (seq_ca); i++)
    if (c[i] == a[seq_ca[j]]) ceq[i] |= 2, aeq[seq_ca[j]] |= 1, j++;

  //werror ("%O\n", ({aeq, beq, ceq}));

  array(array) ares = ({}), bres = ({}), cres = ({});
  int ai = 0, bi = 0, ci = 0;
  int prevodd = -2;

  while (!(aeq[ai] & beq[bi] & ceq[ci] & 4)) {
    //werror ("aeq[%d]=%d beq[%d]=%d ceq[%d]=%d prevodd=%d\n",
    //    ai, aeq[ai], bi, beq[bi], ci, ceq[ci], prevodd);
    array empty = ({}), apart = empty, bpart = empty, cpart = empty;
    int side = aeq[ai] & beq[bi] & ceq[ci];

    if ((<1, 2>)[side]) {
      // Got cyclically interlocking equivalences. Have to break one
      // of them. Prefer the shortest.
      int which, merge, inv_side = side ^ 3, i, oi;
      array(int) eq, oeq;
      array arr, oarr;
      int atest = side == 1 ? ceq[ci] != 3 : beq[bi] != 3;
      int btest = side == 1 ? aeq[ai] != 3 : ceq[ci] != 3;
      int ctest = side == 1 ? beq[bi] != 3 : aeq[ai] != 3;

      for (i = 0;; i++) {
	int abreak = atest && aeq[ai] != aeq[ai + i];
	int bbreak = btest && beq[bi] != beq[bi + i];
	int cbreak = ctest && ceq[ci] != ceq[ci + i];

	if (abreak + bbreak + cbreak > 1) {
	  // More than one shortest sequence. Avoid breaking one that
	  // could give an all-three match later.
	  if (side == 1) {
	    if (!atest) cbreak = 0;
	    if (!btest) abreak = 0;
	    if (!ctest) bbreak = 0;
	  }
	  else {
	    if (!atest) bbreak = 0;
	    if (!btest) cbreak = 0;
	    if (!ctest) abreak = 0;
	  }
	  // Prefer breaking one that can be joined with the previous
	  // diff part.
	  switch (prevodd) {
	    case 0: if (abreak) bbreak = cbreak = 0; break;
	    case 1: if (bbreak) cbreak = abreak = 0; break;
	    case 2: if (cbreak) abreak = bbreak = 0; break;
	  }
	}

	if (abreak) {
	  which = 0, merge = (<0, -1>)[prevodd];
	  i = ai, eq = aeq, arr = a;
	  if (inv_side == 1) oi = bi, oeq = beq, oarr = b;
	  else oi = ci, oeq = ceq, oarr = c;
	  break;
	}
	if (bbreak) {
	  which = 1, merge = (<1, -1>)[prevodd];
	  i = bi, eq = beq, arr = b;
	  if (inv_side == 1) oi = ci, oeq = ceq, oarr = c;
	  else oi = ai, oeq = aeq, oarr = a;
	  break;
	}
	if (cbreak) {
	  which = 2, merge = (<2, -1>)[prevodd];
	  i = ci, eq = ceq, arr = c;
	  if (inv_side == 1) oi = ai, oeq = aeq, oarr = a;
	  else oi = bi, oeq = beq, oarr = b;
	  break;
	}
      }
      //werror ("  which=%d merge=%d inv_side=%d i=%d oi=%d\n",
      //      which, merge, inv_side, i, oi);

      int s = i, mask = eq[i];
      do {
	eq[i++] &= inv_side;
	while (!(oeq[oi] & inv_side)) oi++;
	oeq[oi] &= side;
      }
      while (eq[i] == mask);

      if (merge && !eq[s]) {
	array part = ({});
	do part += ({arr[s++]}); while (!eq[s]);
	switch (which) {
	  case 0: ai = s; ares[-1] += part; break;
	  case 1: bi = s; bres[-1] += part; break;
	  case 2: ci = s; cres[-1] += part; break;
	}
      }
    }
    //werror ("aeq[%d]=%d beq[%d]=%d ceq[%d]=%d prevodd=%d\n",
    //    ai, aeq[ai], bi, beq[bi], ci, ceq[ci], prevodd);

    if (aeq[ai] == 2 && beq[bi] == 1) { // a and b are equal.
      do apart += ({a[ai++]}), bi++; while (aeq[ai] == 2 && beq[bi] == 1);
      bpart = apart;
      while (!ceq[ci]) cpart += ({c[ci++]});
      prevodd = 2;
    }
    else if (beq[bi] == 2 && ceq[ci] == 1) { // b and c are equal.
      do bpart += ({b[bi++]}), ci++; while (beq[bi] == 2 && ceq[ci] == 1);
      cpart = bpart;
      while (!aeq[ai]) apart += ({a[ai++]});
      prevodd = 0;
    }
    else if (ceq[ci] == 2 && aeq[ai] == 1) { // c and a are equal.
      do cpart += ({c[ci++]}), ai++; while (ceq[ci] == 2 && aeq[ai] == 1);
      apart = cpart;
      while (!beq[bi]) bpart += ({b[bi++]});
      prevodd = 1;
    }

    else if ((<1*2*3, 3*3*3>)[aeq[ai] * beq[bi] * ceq[ci]]) { // All are equal.
      // Got to match both when all three are 3 and when they are 1, 2
      // and 3 in that order modulo rotation (might get such sequences
      // after breaking the cyclic equivalences above).
      do apart += ({a[ai++]}), bi++, ci++;
      while ((<0333, 0123, 0312, 0231>)[aeq[ai] << 6 | beq[bi] << 3 | ceq[ci]]);
      cpart = bpart = apart;
      prevodd = -1;
    }

    else {
      // Haven't got any equivalences in this block. Avoid adjacent
      // complementary blocks (e.g. ({({"foo"}),({}),({})}) next to
      // ({({}),({"bar"}),({"bar"})})). Besides that, leave the
      // odd-one-out sequence empty in a block where two are equal.
      switch (prevodd) {
	case 0: apart = ares[-1], ares[-1] = ({}); break;
	case 1: bpart = bres[-1], bres[-1] = ({}); break;
	case 2: cpart = cres[-1], cres[-1] = ({}); break;
      }
      prevodd = -1;
      while (!aeq[ai]) apart += ({a[ai++]});
      while (!beq[bi]) bpart += ({b[bi++]});
      while (!ceq[ci]) cpart += ({c[ci++]});
    }

    //werror ("%O\n", ({apart, bpart, cpart}));
    ares += ({apart}), bres += ({bpart}), cres += ({cpart});
  }

  return ({ares, bres, cres});
}

// diff3, complement to diff (alpha stage)

array(array(array)) diff3_old(array mid,array left,array right)
{
   array(array) lmid,ldst;
   array(array) rmid,rdst;

   [lmid,ldst]=diff(mid,left);
   [rmid,rdst]=diff(mid,right);

   int l=0,r=0,n;
   array(array(array)) res=({});
   int lpos=0,rpos=0;
   array eq=({});
   int x;

   for (n=0; ;)
   {
      while (l<sizeof(lmid) && lpos>=sizeof(lmid[l]))
      {
	 if (sizeof(ldst[l])>lpos)
	    res+=({({({}),ldst[l][lpos..],({})})});
	 l++;
	 lpos=0;
      }
      while (r<sizeof(rmid) && rpos>=sizeof(rmid[r])) 
      {
	 if (sizeof(rdst[r])>rpos)
	    res+=({({({}),({}),rdst[r][rpos..]})});
	 r++;
	 rpos=0;
      }

      if (n==sizeof(mid)) break;

      x=min(sizeof(lmid[l])-lpos,sizeof(rmid[r])-rpos);

      if (lmid[l]==ldst[l] && rmid[r]==rdst[r])
      {
	 eq=lmid[l][lpos..lpos+x-1];
	 res+=({({eq,eq,eq})});
      }
      else if (lmid[l]==ldst[l])
      {
	 eq=lmid[l][lpos..lpos+x-1];
	 res+=({({eq,eq,rdst[r][rpos..rpos+x-1]})});
      }
      else if (rmid[r]==rdst[r])
      {
	 eq=rmid[r][rpos..rpos+x-1];
	 res+=({({eq,ldst[l][lpos..lpos+x-1],eq})});
      }
      else 
      {
	 res+=({({lmid[l][lpos..lpos+x-1],
		  ldst[l][lpos..lpos+x-1],
		  rdst[r][rpos..rpos+x-1]})});
      }

//        werror(sprintf("-> %-5{%O%} %-5{%O%} %-5{%O%}"
//  		     " x=%d l=%d:%d r=%d:%d \n",@res[-1],x,l,lpos,r,rpos));
	 
      rpos+=x;
      lpos+=x;
      n+=x;
   }
   
   return transpose(res);
}

// sort with care of numerical sort:
//  "abc4def" before "abc30def"

int dwim_sort_func(string a0,string b0)
{
   string a2="",b2="";
   int a1,b1;
   sscanf(a0,"%s%d%s",a0,a1,a2);
   sscanf(b0,"%s%d%s",b0,b1,b2);
   if (a0>b0) return 1;
   if (a0<b0) return 0;
   if (a1>b1) return 1;
   if (a1<b1) return 0;
   if (a2==b2) return 0;
   return dwim_sort_func(a2,b2);
}

// sort with no notice of contents in paranthesis,
// no care of case either

int lyskom_sort_func(string a,string b)
{
   string a0=a,b0=b;
   a=replace(lower_case(a),"][\\}{|"/1,"������"/1);
   b=replace(lower_case(b),"][\\}{|"/1,"������"/1);
   
   while (sscanf(a0=a,"%*[ \t](%*[^)])%*[ \t]%s",a)==4 && a0!=a);
   while (sscanf(b0=b,"%*[ \t](%*[^)])%*[ \t]%s",b)==4 && b0!=b);
   a0=b0="";
   sscanf(a,"%[^ \t]%*[ \t](%*[^)])%*[ \t]%s",a,a0);
   sscanf(b,"%[^ \t]%*[ \t](%*[^)])%*[ \t]%s",b,b0);
   if (a>b) return 1;
   if (a<b) return 0;
   if (a0==b0) return 0;
   return lyskom_sort_func(a0,b0);
}
