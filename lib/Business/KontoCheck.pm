#vim:set tw=70;set ft=text; set si

package Business::KontoCheck;

use 5.008008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

our @EXPORT_OK = qw( kto_check kto_check_str generate_lut
      konto_check_at kto_check_at_str generate_lut_at %kto_retval );

our @EXPORT = qw( kto_check kto_check_str kto_check_at
      kto_check_at_str %kto_retval );

our $VERSION = '2.2';

require XSLoader;
XSLoader::load('Business::KontoCheck', $VERSION);

# Preloaded methods go here.

%Business::KontoCheck::kto_retval = (
 -29  =>  'Die Untermethode ist nicht definiert',
 -28  =>  'Die Funktion wurde beim Kompilieren ausgeschlossen',
 -27  =>  'Die Versionsnummer für die LUT-Datei ist ungültig',
 -26  =>  'ungültiger Prüfparameter (erste zu prüfende Stelle)',
 -25  =>  'ungültiger Prüfparameter (Anzahl zu prüfender Stellen)',
 -24  =>  'ungültiger Prüfparameter (Position der Prüfziffer)',
 -23  =>  'ungültiger Prüfparameter (Wichtung)',
 -22  =>  'ungültiger Prüfparameter (Rechenmethode)',
 -21  =>  'Problem beim Initialisieren der globalen Variablen',
 -20  =>  'Prüfsummenfehler in der blz.lut Datei',
 -19  =>  'Die Bankleitzahl wurde gelöscht; das Konto ist falsch',
 -18  =>  'Die Bankleitzahl wurde gelöscht; das Konto ist richtig',
 -17  =>  'Die Bankleitzahl wurde gelöscht; das Konto ist richtig, ohne Prüfung',
 -16  =>  'Die Bankleitzahl wurde gelöscht',
 -15  =>  'Fehler in der blz.txt Datei (falsche Zeilenlänge)',
 -14  =>  'undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert',
 -13  =>  'schwerer Fehler im KontoCheck-Modul',
 -12  =>  'ein Konto muß zwischen 1 und 10 Stellen haben',
 -11  =>  'kann Datei nicht schreiben',
 -10  =>  'kann Datei nicht lesen',
  -9  =>  'kann keinen Speicher allokieren',
  -8  =>  'die Datei mit den Bankleitzahlen wurde nicht gefunden',
  -7  =>  'die blz.lut Datei ist inkosistent/ungültig',
  -6  =>  'die blz.lut Datei wurde nicht gefunden',
  -5  =>  'die Bankleitzahl ist nicht achtstellig',
  -4  =>  'die Bankleitzahl ist ungültig',
  -3  =>  'das Konto ist ungültig',
  -2  =>  'die Methode wurde noch nicht implementiert',
  -1  =>  'die Methode ist nicht definiert',
   0  =>  'falsch',
   1  =>  'ok',
   2  =>  'ok, ohne Prüfung',

 'UNDEFINED_SUBMETHOD'           =>  'Die Untermethode ist nicht definiert',
 'EXCLUDED_AT_COMPILETIME'       =>  'Die Funktion wurde beim Kompilieren ausgeschlossen',
 'INVALID_LUT_VERSION'           =>  'Die Versionsnummer für die LUT-Datei ist ungültig',
 'INVALID_PARAMETER_STELLE1'     =>  'ungültiger Prüfparameter (erste zu prüfende Stelle)',
 'INVALID_PARAMETER_COUNT'       =>  'ungültiger Prüfparameter (Anzahl zu prüfender Stellen)',
 'INVALID_PARAMETER_PRUEFZIFFER' =>  'ungültiger Prüfparameter (Position der Prüfziffer)',
 'INVALID_PARAMETER_WICHTUNG'    =>  'ungültiger Prüfparameter (Wichtung)',
 'INVALID_PARAMETER_METHODE'     =>  'ungültiger Prüfparameter (Rechenmethode)',
 'LIBRARY_INIT_ERROR'            =>  'Problem beim Initialisieren der globalen Variablen',
 'FALSE_GELOESCHT'               =>  'Die Bankleitzahl wurde gelöscht; das Konto ist falsch',
 'OK_NO_CHK_GELOESCHT'           =>  'Die Bankleitzahl wurde gelöscht; das Konto ist richtig',
 'OK_GELOESCHT'                  =>  'Die Bankleitzahl wurde gelöscht; das Konto ist richtig, ohne Prüfung',
 'BLZ_GELOESCHT'                 =>  'Die Bankleitzahl wurde gelöscht',
 'INVALID_BLZ_FILE'              =>  'Fehler in der blz.txt Datei (falsche Zeilenlänge)',
 'LIBRARY_IS_NOT_THREAD_SAFE'    =>  'undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert',
 'FATAL_ERROR'                   =>  'schwerer Fehler im Konto-Modul',
 'INVALID_KTO_LENGTH'            =>  'ein Konto muß zwischen 1 und 10 Stellen haben',
 'FILE_WRITE_ERROR'              =>  'kann Datei nicht schreiben',
 'FILE_READ_ERROR'               =>  'kann Datei nicht lesen',
 'ERROR_MALLOC'                  =>  'kann keinen Speicher allokieren',
 'NO_BLZ_FILE'                   =>  'die Datei mit den Bankleitzahlen wurde nicht gefunden',
 'INVALID_LUT_FILE'              =>  'die blz.lut Datei ist inkosistent/ungültig',
 'NO_LUT_FILE'                   =>  'die blz.lut Datei wurde nicht gefunden',
 'INVALID_BLZ_LENGTH'            =>  'die Bankleitzahl ist nicht achtstellig',
 'INVALID_BLZ'                   =>  'die Bankleitzahl ist ungültig',
 'INVALID_KTO'                   =>  'das Konto ist ungültig',
 'NOT_IMPLEMENTED'               =>  'die Methode wurde noch nicht implementiert',
 'NOT_DEFINED'                   =>  'die Methode ist nicht definiert',
 'FALSE'                         =>  'falsch',
 'OK'                            =>  'ok',
 'OK_NO_CHK'                     =>  'ok, ohne Prüfung',
);

1;
__END__

=head1 NAME

Business::KontoCheck - Perl extension for checking German and Austrian Bank Account Numbers

=head1 NOTE

Because the module is for use mainly in Germany, the following documentation
language is german too.

=head1 SYNOPSIS

   use Business::KontoCheck;
   use Business::KontoCheck qw( kto_check kto_check_str generate_lut
         konto_check_at kto_check_at_str generate_lut_at %kto_retval );

   $retval=kto_check($blz,$kto,$lut_name);
   $retval=kto_check_str($blz,$kto,$lut_name);
   $retval=generate_lut($inputname,$outputname,$user_info,$lut_version);
   $retval=kto_check_at($blz,$kto,$lut_name);
   $retval=kto_check_at_str($blz,$kto,$lut_name);
   $retval=generate_lut_at($inputname,$outputname,$plain_name,$plain_format);
   $retval_txt=$kto_retval{$retval};

=head1 DESCRIPTION

  Funktion:  kto_check()
             kto_check_str()

  Aufgabe:   Testen eines Kontos

  Aufruf:    $retval=kto_check($blz,$kto,$lut_name);
             $retval=kto_check_str($blz,$kto,$lut_name);

  Parameter:
     $blz:      falls 2- oder 3-stellig: Prüfziffermethode
                (evl. mit Untermethode a, b, c... oder 1, 2, 3)
                falls 8-stellig: Bankleitzahl

     $kto:      Kontonummer (wird vor der Berechnung
                linksbündig mit Nullen auf 10 Stellen
                aufgefüllt)

     $lut_name: Dateiname der Lookup-Tabelle mit Bankleitzahlen.
                Falls NULL oder ein leerer String übergeben wird,
                wird der Dateiname blz.lut benutzt.
                Diese Datei enthält die Prüfziffermethoden für die
                einzelnen Bankleitzahlen; sie kann mit der Funktion
                generate_lut() aus der Bundesbanktabelle generiert
                werden.

  Rückgabewerte:
     Die Funktion kto_check gibt einen numerischen Wert zurück, während die
     Funktion kto_check_str einen kurzen String zurückgibt. Die folgenden
     Werte sind definiert:

          INVALID_LUT_VERSION        -20
          FALSE_GELOESCHT            -19
          OK_NO_CHK_GELOESCHT        -18
          OK_GELOESCHT               -17
          BLZ_GELOESCHT              -16
          INVALID_BLZ_FILE           -15
          LIBRARY_IS_NOT_THREAD_SAFE -14
          FATAL_ERROR                -13
          INVALID_KTO_LENGTH         -12
          FILE_WRITE_ERROR           -11
          FILE_READ_ERROR            -10
          ERROR_MALLOC                -9
          NO_BLZ_FILE                 -8
          INVALID_LUT_FILE            -7
          NO_LUT_FILE                 -6
          INVALID_BLZ_LENGTH          -5
          INVALID_BLZ                 -4
          INVALID_KTO                 -3
          NOT_IMPLEMENTED             -2
          NOT_DEFINED                 -1
          FALSE                        0
          OK                           1
          OK_NO_CHK                    2

     Mittels des assoziativen Arrays %kto_retval lassen sich die numerischen
     und kurzen Rückgabewerte in einen etwas ausführlicheren Rückgabetext
     umwandeln:

     $retval_txt=$kto_retval{$retval};

-------------------------------------------------------------------------

  Funktion:  generate_lut()

  Aufgabe:   LUT-Datei generieren

  Aufruf: $retval=generate_lut($inputname,$outputname,$user_info,$lut_version);

  Parameter:
     inputname:  Name der Bankleitzahlendatei der Deutschen
                 Bundesbank (z.B. blz0303pc.txt)

     outputname: Name der Zieldatei (z.B. blz.lut)

     user_info:  Info-Zeile, die zusätzlich in die LUT-Datei
                 geschrieben wird. Diese Zeile wird von der
                 Funktion get_lut_info() in zurückgegeben,
                 aber ansonsten nicht ausgewertet.


     lut_version: Format der LUT-Datei. Mögliche Werte:
                 1: altes Format (1.0)
                 2: altes Format (1.1) mit Infozeile

  Bugs:
     Diese Funktion sollte nicht von einem Programm aufgerufen werden,
     das zum Testen von Kontoverbindungen benutzt wird, da teilweise
     dieselben Variablen benutzt werden, und so falsche Ergebnisse
     erzeugt werden können. 

  Rückgabewerte:

     NO_BLZ_FILE          Bankleitzahlendatei nicht gefunden
     FILE_WRITE_ERROR     kann Datei nicht schreiben (Schreibschutz?)
     OK                   Erfolg

-------------------------------------------------------------------------

  Funktion:  kto_check_at()
             kto_check_at_str()

  Aufgabe:   Testen eines österreichischen Kontos

  Aufruf:    $retval=kto_check_at($blz,$kto,$lut_name);
             $retval=kto_check_at_str($blz,$kto,$lut_name);

  Parameter:
  $blz:     BLZ (5-stellig) oder Prüfparameter (mit vorangestelltem p)
            Falls der BLZ ein - vorausgestellt wird, werden auch gelöschte
            Bankleitzahlen geprüft.
            Falls der BLZ ein p vorausgestellt wird, wird der folgende
            Teil (bis zum Blank/Tab) als Prüfparameter angesehen.

  $kto:     Kontonummer

  $lut_name: Name der Lookup-Datei oder Leerstring
            Falls für $lut_name ein Leerstring angegeben wird, versucht
            die Funktion, die Datei blz-at.lut zu lesen.

  Rückgabewerte:
            s.o. (wie bei der deutschen Version)

-------------------------------------------------------------------------

  Funktion:  generate_lut_at()

  Aufgabe:   LUT-Datei für das österreichische Modul generieren

  Aufruf:    $retval=generate_lut_at($inputname,$outputname,$plain_name,$plain_format);

  Parameter:
     $inputname:  Name der INPAR-Datei (nur komplett, nicht inkrementell!)
     $outputname: Name der Zieldatei (z.B. blz-at.lut)
     $plain_name: (optional) Name einer Ausgabedatei für die Klartextausgabe.
     $plain_format: Format der Klartextausgabe (s.u.)

  Bugs:
     Diese Funktion sollte nicht von einem Programm aufgerufen werden,
     das zum Testen von Kontoverbindungen benutzt wird, da teilweise
     dieselben Variablen benutzt werden, und so falsche Ergebnisse
     erzeugt werden können. 

  Die Funktion generate_lut_at() generiert aus der Institutsparameter-
  Datenbankdatei (5,3 MB) eine kleine Datei (8,3 KB), in der nur die
  Bankleitzahlen und Prüfziffermethoden gespeichert sind. Um die Datei
  klein zu halten, wird der größte Teil der Datei binär gespeichert.

  Falls der Parameter plain_name angegeben wird, wird zu jeder INPAR-
  Eintrag außerdem (in einem frei wählbaren Format) noch in eine Klartext-
  datei geschrieben. Das Format der Datei wird durch den 4. Parameter
  ($plain_format) bestimmt. Es sind die folgenden Felder und Escape-
  Sequenzen definiert (der Sortierparameter muß als erstes Zeichen
  kommen!):

     @i   Sortierung nach Identnummern
     @b   Sortierung nach Bankleitzahlen (default)
     %b   Bankleitzahl
     %B   Bankleitzahl (5-stellig, links mit Nullen aufgefüllt)
     %f   Kennzeichen fiktive Bankleitzahl
     %h   Kennzeichen Hauptstelle/Zweigstelle
     %i   Identnummer der Österreichischen Nationalbank
     %I   Identnummer der Österreichischen Nationalbank (7-stellig)
     %l   Löschdatum (DD.MM.YYYY falls vorhanden, sonst nichts)
     %L   Löschdatum (DD.MM.YYYY falls vorhanden, sonst 10 Blanks)
     %n1  Erster Teil des Banknamens
     %n2  Zweiter Teil des Banknamens
     %n3  Dritter Teil des Banknamens
     %N   kompletter Bankname (alle drei Teile zusammengesetzt)
     %p   Kontoprüfparameter
     %t   Name der Prüftabelle
     %z   zugeordnete BLZ (nur bei fiktiver BLZ, sonst nichts)
     %Z   zugeordnete BLZ (5-stellig bei fiktiver BLZ, sonst 5 Blanks)
     %%   das % Zeichen selbst

     \n   Zeilenvorschub
     \r   CR (für M$DOS)
     \t   Tabulatorzeichen
     \\   ein \

  @i (bzw. @b) muß am Anfang des Formatstrings stehen; falls keine
  Sortierung angegeben wird, wird @b benutzt.

  Nicht definierte Felder und Escape-Sequenzen werden (zumindest momentan
  noch) direkt in die Ausgabedatei übernommen. D.h., wenn man %x schreibt,
  erscheint in der Ausgabedatei auch ein %x, ohne daß ein Fehler gemeldet
  wird. Ob dies ein Bug oder Feature ist, sei dahingestellt; momentan
  scheint es eher ein Feature zu sein ;-))).

  Falls kein plain_format angegeben wird, wird "@B%I %B %t %N"
  benutzt. Die Datei ist (anders als die INPAR-Datei) nach
  Bankleitzahlen sortiert. Nähres zur Sortierung findet sich in der
  Einleitung zur Funktion cmp_blz().

  Die Funktion ist **nicht** threadfest, da dies aufgrund der gewählten
  Implementierung nur schwer zu machen wäre, und auch nicht sehr sinnvoll
  ist (sie wird nur benötigt, um die blz-at.lut Datei zu erstellen).

-------------------------------------------------------------------------


=head1 DESCRIPTION

Dies ist Business::KontoCheck, ein Programm zum Testen der
Prüfziffern von deutschen und österreichischen Bankkonten. Es war
ursprünglich für die Benutzung mit dem dtaus-Paket von Martin
Schulze <joey@infodrom.org> und dem lx2l Präprozessor gedacht;
es läßt sich jedoch auch mit beliebigen anderen Programmen
verwenden. Dies ist die Perl-Version der C-Library.

=head1 EXPORT

Es werden defaultmäßig die Funkionen kto_check und kto_check_str,
(aus dem deutschen Modul), kto_check_at, kto_check_at_str (aus
dem österreichischen Modul) sowie die Variable %kto_retval (für
beide Module) exportiert.

Optional können auch die Funktionen generate_lut, sowie
generate_lut_at exportiert werden; in diesem Fall sind die gewünschten
Funktionen in der use Klausel anzugeben.

=head1 KNOWN BUGS:

Die Funktionen generate_lut() und generate_lut_at() sollte nicht von
einem Programm aufgerufen werden, das zum Testen von Kontoverbindungen
benutzt wird, da teilweise dieselben Variablen benutzt werden, und so
falsche Ergebnisse erzeugt werden können. 

Die Bibliothek ist momentan nicht threadfest, da einige globale
Variablen benutzt werden; die nächsten Version wird eine threadfeste
Variante enthalten (der kleine Geschwindigkeitsvorteil macht die
Nachteile nicht wett).

Momentan gibt es für langlaufende Serveranwendungen noch keine
elegante Methode, eine neue LUT-Datei zu aktivieren; das wird auch im
Zusammenhang mit der Threadfestigkeit in Angriff genommen werden. Es
wäre möglich dafür die Funktion cleanup_kto() zu benutzen; diese
enthält allerdings keine locking Funktionalität, um einen Zugriff von
anderen Threads zu verhindern; daher wurde die Funktion nicht
exportiert. Momentan scheint der beste Weg zu sein, den entsprechenden
Server neu zu starten :-( - nicht schön, aber sauber.


=head1 SEE ALSO

Eine ausführliche Beschreibung der Prüfziffermethoden und das Format der
LUT-Datei findet sich im C-Quellcode.

Momentan gibt es noch keine mailingliste; es wird allerdings bald eine
solche eingerichtet werden.

Die aktuelle Version findet sich unter http://www.informatik.hs-mannheim.de/konto_check

=head1 AUTHOR

Michael Plugge, E<lt>m.plugge@hs-mannheim.deE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 by Michael Plugge

This library is free software; you can redistribute it and/or modify it under
the same terms as Perl itself, either Perl version 5.8.8 or, at your option,
any later version of Perl 5 you may have available (perl and glue code).

The C library is covered by the GNU Lesser General Public License:

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option) any
later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

=cut
