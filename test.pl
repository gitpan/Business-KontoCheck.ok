# Dies ist eine kleine Beispielsanwendung für Business::KontoCheck. Das Programm
# list eine Reihe Bankleitzahlen und Kontonummern (durch mindestens ein nicht-alphanumerisches
# Zeichen getrennt) sowie noch evl. nachfolgenden Kommentar ein, testet das Konto auf
# Gültigkeit und gibt die Zeile (ergänzt durch den Rückgabewert) wieder aus. Leerzeilen sowie
# Zeilen ohne Bankleitzahl oder Kontonummer werden unverändert ausgegeben.
#
# Geschrieben 9.6.07, Michael Plugge

use Business::KontoCheck;

open(IN,"testkonten.txt") or die "Kann testkonten.txt nicht öffenen: $!\n";
open(OUT,"> testkonten.out") or die "Kann testkonten.out nicht öffenen: $!\n";

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

print OUT "\n===============================================\n\nÖsterreichische Testkonten:\n\n";
open(IN,"testkonten-at.txt") or die "Kann testkonten-at.txt nicht öffenen: $!\n";

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

