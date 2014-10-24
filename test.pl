# Dies ist eine kleine Beispielsanwendung f�r Business::KontoCheck. Das Programm
# list eine Reihe Bankleitzahlen und Kontonummern (durch mindestens ein nicht-alphanumerisches
# Zeichen getrennt) sowie noch evl. nachfolgenden Kommentar ein, testet das Konto auf
# G�ltigkeit und gibt die Zeile (erg�nzt durch den R�ckgabewert) wieder aus. Leerzeilen sowie
# Zeilen ohne Bankleitzahl oder Kontonummer werden unver�ndert ausgegeben.
#
# Geschrieben 9.6.07, Michael Plugge

use Business::KontoCheck;

open(IN,"testkonten.txt") or die "Kann testkonten.txt nicht �ffenen: $!\n";
open(OUT,"> testkonten.out") or die "Kann testkonten.out nicht �ffenen: $!\n";

while(<IN>){
   chomp;
   ($valid,$blz,$separator,$kto,$rest)=/(([0-9a-zA-Z\-]+)([^0-9a-zA-Z]+)([0-9]+))?(.*)/;
   if($valid){
      $retval=kto_check($blz,$kto,"");
      print OUT "$blz$separator$kto$rest: $kto_retval{$retval}\n";
   }
   else{
      print OUT "$rest\n";
   }
}

print OUT "\n===============================================\n\n�sterreichische Testkonten:\n\n";
open(IN,"testkonten-at.txt") or die "Kann testkonten-at.txt nicht �ffenen: $!\n";

while(<IN>){
   chomp;
   ($valid,$blz,$separator,$kto,$rest)=/(([0-9a-zA-Z\-]+)([^0-9a-zA-Z]+)([0-9]+))?(.*)/;
   if($valid){
      $retval=kto_check_at($blz,$kto,"");
      print OUT "$blz$separator$kto$rest: $kto_retval{$retval}\n";
   }
   else{
      print OUT "$rest\n";
   }
}

