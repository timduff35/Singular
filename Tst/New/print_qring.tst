LIB "tst.lib"; tst_init();

proc TestRingPrinting(def r, string n)
{
  def save = basering;
  
  setring r;
  
  "VVVVVVVVVVVV[", n, "]VVVVVVVVVVVV";
  
  "r == currRing (the following type and print should yield the same output!): ";
  "type: "; r;
  "print(ring): "; print(r);  

  ring @temp = 2,@a,ds; setring @temp;
  
  "r != currRing (the following type and print may be different!): ";  
  "type: "; r;
  "print(ring): "; print(r);
  
  "^^^^^^^^^^^^[", n, "]^^^^^^^^^^^^";
  kill @temp;
  
  setring save;
}

ring r; 

TestRingPrinting(r, "default commutative polynomial ring");

qring q = std(ideal(x2));

TestRingPrinting(q, "quotient ring");

tst_status(1);$
