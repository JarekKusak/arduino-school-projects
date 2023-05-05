Emulátor hrací kostky pro hru Dungeons&Dragons - Jaroslav Kusák

Obecná charakteristika

Program emuluje hrací kostku ve hře Dungeons&Dragons, přičemž první tlačítko generuje náhodné číslo (prvek náhody je závislý na době stlačeného tlačítka) a následně ho zobrazuje na sedmisegmentový displej. Během generování je na displeji zobrazována jednoduchá animace měnících se tří náhodných znaků, přičemž do toho probíhá ještě svícení ledek, které se rozsvicují a zhasínají (odrážejí se od okrajů, tak jak kdysi v předchozí úloze). Druhé a třetí tlačítko přepíná do konfiguračního módu, přičemž je nastavitelný počet hodů kostkou a její počet stěn (náhodné číslo je pak součtem náhodných hodů).

Objektová dekompozice

Program sestává ze tří tříd: Button, Display, Dice. Každá třída se stará o to, oco její název napovídá. Třída Button je zodpovědná za inicializaci a kontrolu stisknutí (případně držení) jednotlivých tlačítek, třída Display obsluhuje práci se sedmi segmentovým displejem a LEDky a třída Dice má za úkol simulovat hrací kostku, tedy generovat náhodná čísla. 

Při rozhodování, jak co naprogramovat, byla jednoznačná volba objektové programování, jednotlivé části funshieldu oddělit do tříd a rozdělit logiku mezi jednotlivé komponenty funshieldu. Při realizaci projektu se nevyskytly přílišné problémy, jediný problém přišel, když jsem chtěl přidat svou vlastní "featuru", a to využití piezza, které by hrálo nějakou pěknou muziku během generování. Bohužel z tohoto nápadu sešlo, jelikož je práce s piezzem (tedy spíše s hudbou) docela obtížná a nevěděl jsem, jak to zrealizovat bez využití funkce delay(). 

Vlastní featura

Mou vlastní "featurou" je rozsvicování ledek při generování náhodného čísla. Nejedná se o nic složitého, ale přesto to k animaci na displeji přidává o něco hezčí "vizuální podívanou"... 
