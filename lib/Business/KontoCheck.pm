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
 -27  =>  'Die Versionsnummer f�r die LUT-Datei ist ung�ltig',
 -26  =>  'ung�ltiger Pr�fparameter (erste zu pr�fende Stelle)',
 -25  =>  'ung�ltiger Pr�fparameter (Anzahl zu pr�fender Stellen)',
 -24  =>  'ung�ltiger Pr�fparameter (Position der Pr�fziffer)',
 -23  =>  'ung�ltiger Pr�fparameter (Wichtung)',
 -22  =>  'ung�ltiger Pr�fparameter (Rechenmethode)',
 -21  =>  'Problem beim Initialisieren der globalen Variablen',
 -20  =>  'Pr�fsummenfehler in der blz.lut Datei',
 -19  =>  'Die Bankleitzahl wurde gel�scht; das Konto ist falsch',
 -18  =>  'Die Bankleitzahl wurde gel�scht; das Konto ist richtig',
 -17  =>  'Die Bankleitzahl wurde gel�scht; das Konto ist richtig, ohne Pr�fung',
 -16  =>  'Die Bankleitzahl wurde gel�scht',
 -15  =>  'Fehler in der blz.txt Datei (falsche Zeilenl�nge)',
 -14  =>  'undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert',
 -13  =>  'schwerer Fehler im KontoCheck-Modul',
 -12  =>  'ein Konto mu� zwischen 1 und 10 Stellen haben',
 -11  =>  'kann Datei nicht schreiben',
 -10  =>  'kann Datei nicht lesen',
  -9  =>  'kann keinen Speicher allokieren',
  -8  =>  'die Datei mit den Bankleitzahlen wurde nicht gefunden',
  -7  =>  'die blz.lut Datei ist inkosistent/ung�ltig',
  -6  =>  'die blz.lut Datei wurde nicht gefunden',
  -5  =>  'die Bankleitzahl ist nicht achtstellig',
  -4  =>  'die Bankleitzahl ist ung�ltig',
  -3  =>  'das Konto ist ung�ltig',
  -2  =>  'die Methode wurde noch nicht implementiert',
  -1  =>  'die Methode ist nicht definiert',
   0  =>  'falsch',
   1  =>  'ok',
   2  =>  'ok, ohne Pr�fung',

 'UNDEFINED_SUBMETHOD'           =>  'Die Untermethode ist nicht definiert',
 'EXCLUDED_AT_COMPILETIME'       =>  'Die Funktion wurde beim Kompilieren ausgeschlossen',
 'INVALID_LUT_VERSION'           =>  'Die Versionsnummer f�r die LUT-Datei ist ung�ltig',
 'INVALID_PARAMETER_STELLE1'     =>  'ung�ltiger Pr�fparameter (erste zu pr�fende Stelle)',
 'INVALID_PARAMETER_COUNT'       =>  'ung�ltiger Pr�fparameter (Anzahl zu pr�fender Stellen)',
 'INVALID_PARAMETER_PRUEFZIFFER' =>  'ung�ltiger Pr�fparameter (Position der Pr�fziffer)',
 'INVALID_PARAMETER_WICHTUNG'    =>  'ung�ltiger Pr�fparameter (Wichtung)',
 'INVALID_PARAMETER_METHODE'     =>  'ung�ltiger Pr�fparameter (Rechenmethode)',
 'LIBRARY_INIT_ERROR'            =>  'Problem beim Initialisieren der globalen Variablen',
 'FALSE_GELOESCHT'               =>  'Die Bankleitzahl wurde gel�scht; das Konto ist falsch',
 'OK_NO_CHK_GELOESCHT'           =>  'Die Bankleitzahl wurde gel�scht; das Konto ist richtig',
 'OK_GELOESCHT'                  =>  'Die Bankleitzahl wurde gel�scht; das Konto ist richtig, ohne Pr�fung',
 'BLZ_GELOESCHT'                 =>  'Die Bankleitzahl wurde gel�scht',
 'INVALID_BLZ_FILE'              =>  'Fehler in der blz.txt Datei (falsche Zeilenl�nge)',
 'LIBRARY_IS_NOT_THREAD_SAFE'    =>  'undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert',
 'FATAL_ERROR'                   =>  'schwerer Fehler im Konto-Modul',
 'INVALID_KTO_LENGTH'            =>  'ein Konto mu� zwischen 1 und 10 Stellen haben',
 'FILE_WRITE_ERROR'              =>  'kann Datei nicht schreiben',
 'FILE_READ_ERROR'               =>  'kann Datei nicht lesen',
 'ERROR_MALLOC'                  =>  'kann keinen Speicher allokieren',
 'NO_BLZ_FILE'                   =>  'die Datei mit den Bankleitzahlen wurde nicht gefunden',
 'INVALID_LUT_FILE'              =>  'die blz.lut Datei ist inkosistent/ung�ltig',
 'NO_LUT_FILE'                   =>  'die blz.lut Datei wurde nicht gefunden',
 'INVALID_BLZ_LENGTH'            =>  'die Bankleitzahl ist nicht achtstellig',
 'INVALID_BLZ'                   =>  'die Bankleitzahl ist ung�ltig',
 'INVALID_KTO'                   =>  'das Konto ist ung�ltig',
 'NOT_IMPLEMENTED'               =>  'die Methode wurde noch nicht implementiert',
 'NOT_DEFINED'                   =>  'die Methode ist nicht definiert',
 'FALSE'                         =>  'falsch',
 'OK'                            =>  'ok',
 'OK_NO_CHK'                     =>  'ok, ohne Pr�fung',
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
     $blz:      falls 2- oder 3-stellig: Pr�fziffermethode
                (evl. mit Untermethode a, b, c... oder 1, 2, 3)
                falls 8-stellig: Bankleitzahl

     $kto:      Kontonummer (wird vor der Berechnung
                linksb�ndig mit Nullen auf 10 Stellen
                aufgef�llt)

     $lut_name: Dateiname der Lookup-Tabelle mit Bankleitzahlen.
                Falls NULL oder ein leerer String �bergeben wird,
                wird der Dateiname blz.lut benutzt.
                Diese Datei enth�lt die Pr�fziffermethoden f�r die
                einzelnen Bankleitzahlen; sie kann mit der Funktion
                generate_lut() aus der Bundesbanktabelle generiert
                werden.

  R�ckgabewerte:
     Die Funktion kto_check gibt einen numerischen Wert zur�ck, w�hrend die
     Funktion kto_check_str einen kurzen String zur�ckgibt. Die folgenden
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
     und kurzen R�ckgabewerte in einen etwas ausf�hrlicheren R�ckgabetext
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

     user_info:  Info-Zeile, die zus�tzlich in die LUT-Datei
                 geschrieben wird. Diese Zeile wird von der
                 Funktion get_lut_info() in zur�ckgegeben,
                 aber ansonsten nicht ausgewertet.


     lut_version: Format der LUT-Datei. M�gliche Werte:
                 1: altes Format (1.0)
                 2: altes Format (1.1) mit Infozeile

  Bugs:
     Diese Funktion sollte nicht von einem Programm aufgerufen werden,
     das zum Testen von Kontoverbindungen benutzt wird, da teilweise
     dieselben Variablen benutzt werden, und so falsche Ergebnisse
     erzeugt werden k�nnen. 

  R�ckgabewerte:

     NO_BLZ_FILE          Bankleitzahlendatei nicht gefunden
     FILE_WRITE_ERROR     kann Datei nicht schreiben (Schreibschutz?)
     OK                   Erfolg

-------------------------------------------------------------------------

  Funktion:  kto_check_at()
             kto_check_at_str()

  Aufgabe:   Testen eines �sterreichischen Kontos

  Aufruf:    $retval=kto_check_at($blz,$kto,$lut_name);
             $retval=kto_check_at_str($blz,$kto,$lut_name);

  Parameter:
  $blz:     BLZ (5-stellig) oder Pr�fparameter (mit vorangestelltem p)
            Falls der BLZ ein - vorausgestellt wird, werden auch gel�schte
            Bankleitzahlen gepr�ft.
            Falls der BLZ ein p vorausgestellt wird, wird der folgende
            Teil (bis zum Blank/Tab) als Pr�fparameter angesehen.

  $kto:     Kontonummer

  $lut_name: Name der Lookup-Datei oder Leerstring
            Falls f�r $lut_name ein Leerstring angegeben wird, versucht
            die Funktion, die Datei blz-at.lut zu lesen.

  R�ckgabewerte:
            s.o. (wie bei der deutschen Version)

-------------------------------------------------------------------------

  Funktion:  generate_lut_at()

  Aufgabe:   LUT-Datei f�r das �sterreichische Modul generieren

  Aufruf:    $retval=generate_lut_at($inputname,$outputname,$plain_name,$plain_format);

  Parameter:
     $inputname:  Name der INPAR-Datei (nur komplett, nicht inkrementell!)
     $outputname: Name der Zieldatei (z.B. blz-at.lut)
     $plain_name: (optional) Name einer Ausgabedatei f�r die Klartextausgabe.
     $plain_format: Format der Klartextausgabe (s.u.)

  Bugs:
     Diese Funktion sollte nicht von einem Programm aufgerufen werden,
     das zum Testen von Kontoverbindungen benutzt wird, da teilweise
     dieselben Variablen benutzt werden, und so falsche Ergebnisse
     erzeugt werden k�nnen. 

  Die Funktion generate_lut_at() generiert aus der Institutsparameter-
  Datenbankdatei (5,3 MB) eine kleine Datei (8,3 KB), in der nur die
  Bankleitzahlen und Pr�fziffermethoden gespeichert sind. Um die Datei
  klein zu halten, wird der gr��te Teil der Datei bin�r gespeichert.

  Falls der Parameter plain_name angegeben wird, wird zu jeder INPAR-
  Eintrag au�erdem (in einem frei w�hlbaren Format) noch in eine Klartext-
  datei geschrieben. Das Format der Datei wird durch den 4. Parameter
  ($plain_format) bestimmt. Es sind die folgenden Felder und Escape-
  Sequenzen definiert (der Sortierparameter mu� als erstes Zeichen
  kommen!):

     @i   Sortierung nach Identnummern
     @b   Sortierung nach Bankleitzahlen (default)
     %b   Bankleitzahl
     %B   Bankleitzahl (5-stellig, links mit Nullen aufgef�llt)
     %f   Kennzeichen fiktive Bankleitzahl
     %h   Kennzeichen Hauptstelle/Zweigstelle
     %i   Identnummer der �sterreichischen Nationalbank
     %I   Identnummer der �sterreichischen Nationalbank (7-stellig)
     %l   L�schdatum (DD.MM.YYYY falls vorhanden, sonst nichts)
     %L   L�schdatum (DD.MM.YYYY falls vorhanden, sonst 10 Blanks)
     %n1  Erster Teil des Banknamens
     %n2  Zweiter Teil des Banknamens
     %n3  Dritter Teil des Banknamens
     %N   kompletter Bankname (alle drei Teile zusammengesetzt)
     %p   Kontopr�fparameter
     %t   Name der Pr�ftabelle
     %z   zugeordnete BLZ (nur bei fiktiver BLZ, sonst nichts)
     %Z   zugeordnete BLZ (5-stellig bei fiktiver BLZ, sonst 5 Blanks)
     %%   das % Zeichen selbst

     \n   Zeilenvorschub
     \r   CR (f�r M$DOS)
     \t   Tabulatorzeichen
     \\   ein \

  @i (bzw. @b) mu� am Anfang des Formatstrings stehen; falls keine
  Sortierung angegeben wird, wird @b benutzt.

  Nicht definierte Felder und Escape-Sequenzen werden (zumindest momentan
  noch) direkt in die Ausgabedatei �bernommen. D.h., wenn man %x schreibt,
  erscheint in der Ausgabedatei auch ein %x, ohne da� ein Fehler gemeldet
  wird. Ob dies ein Bug oder Feature ist, sei dahingestellt; momentan
  scheint es eher ein Feature zu sein ;-))).

  Falls kein plain_format angegeben wird, wird "@B%I %B %t %N"
  benutzt. Die Datei ist (anders als die INPAR-Datei) nach
  Bankleitzahlen sortiert. N�hres zur Sortierung findet sich in der
  Einleitung zur Funktion cmp_blz().

  Die Funktion ist **nicht** threadfest, da dies aufgrund der gew�hlten
  Implementierung nur schwer zu machen w�re, und auch nicht sehr sinnvoll
  ist (sie wird nur ben�tigt, um die blz-at.lut Datei zu erstellen).

-------------------------------------------------------------------------


=head1 DESCRIPTION

Dies ist Business::KontoCheck, ein Programm zum Testen der
Pr�fziffern von deutschen und �sterreichischen Bankkonten. Es war
urspr�nglich f�r die Benutzung mit dem dtaus-Paket von Martin
Schulze <joey@infodrom.org> und dem lx2l Pr�prozessor gedacht;
es l��t sich jedoch auch mit beliebigen anderen Programmen
verwenden. Dies ist die Perl-Version der C-Library.

=head1 EXPORT

Es werden defaultm��ig die Funkionen kto_check und kto_check_str,
(aus dem deutschen Modul), kto_check_at, kto_check_at_str (aus
dem �sterreichischen Modul) sowie die Variable %kto_retval (f�r
beide Module) exportiert.

Optional k�nnen auch die Funktionen generate_lut, sowie
generate_lut_at exportiert werden; in diesem Fall sind die gew�nschten
Funktionen in der use Klausel anzugeben.

=head1 KNOWN BUGS:

Die Funktionen generate_lut() und generate_lut_at() sollte nicht von
einem Programm aufgerufen werden, das zum Testen von Kontoverbindungen
benutzt wird, da teilweise dieselben Variablen benutzt werden, und so
falsche Ergebnisse erzeugt werden k�nnen. 

Die Bibliothek ist momentan nicht threadfest, da einige globale
Variablen benutzt werden; die n�chsten Version wird eine threadfeste
Variante enthalten (der kleine Geschwindigkeitsvorteil macht die
Nachteile nicht wett).

Momentan gibt es f�r langlaufende Serveranwendungen noch keine
elegante Methode, eine neue LUT-Datei zu aktivieren; das wird auch im
Zusammenhang mit der Threadfestigkeit in Angriff genommen werden. Es
w�re m�glich daf�r die Funktion cleanup_kto() zu benutzen; diese
enth�lt allerdings keine locking Funktionalit�t, um einen Zugriff von
anderen Threads zu verhindern; daher wurde die Funktion nicht
exportiert. Momentan scheint der beste Weg zu sein, den entsprechenden
Server neu zu starten :-( - nicht sch�n, aber sauber.


=head1 SEE ALSO

Eine ausf�hrliche Beschreibung der Pr�fziffermethoden und das Format der
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
