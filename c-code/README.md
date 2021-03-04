# Litt om koden
Koden her er en klient-server applikasjon som sammenligner bilder som er lagret på to endesystemer. Implementerer egen flytkontroll som gjør at hastigheten avsender sender i, ikke overstiger
hastigheten serveren klarer å motta på. Gjør dette ved å bruke ACK pakker. 
<br/><br/> Implementerer også en Sliding Window algoritme for gjennoppretingen av pakker som mistes. (vindusstørrelse på 7). <br/> <br/>
Pakkene inneholder: <br/>
● (int) pakningens totale lengde inkludert nyttelasten i byte<br/>
● (unsigned char) sekvensnummer for denne pakken<br/>
● (unsigned char) sekvensnummer for den siste mottatte pakken (ACK)<br/>
● (unsigned char) flagg<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;○ 0x1: 1 hvis denne pakken inneholder data<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;○ 0x2: 1 hvis denne pakken inneholder en ACK<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;○ 0x4: 1 hvis denne pakken lukker tilkoblingen<br/>
● Alle andre biter: 0<br/>
● (unsigned char) ubrukt, må alltid inneholde verdien 0x7f<br/>
● (bytes) nyttelast<br/>
<br/><br/>


## Hvordan kjøre koden

For å kompilere nødvendige filer
```bash 
make all
```

For å fjerne filene som blir laget ved kompilering
```bash 
make all
```
Kjøringen av serveren
```bash
./server portnummer_som_serveren_skal_motta_pakker_til  navn_til_mappe_med_bilder  filnavn_for_utskrift
```
Kjøring av klient 
```bash
./klient ip_adresse/hostname_til_maskinen_som_kjører_server  portnummer_som_server_mottar_pakker_til  filnavn_til_liste_med_navn tapsprosen(mellom 0 - 20)
```
__big_set__ er eksempel på mappe med bilder servern kan bruke <br/>
__list_of_filenames__ er eksemple på filnavn som inneholder alle filnavnene til big_set som kan brukes i klienten <br/><br/>
__Serveren må startes opp før serveren__ 
