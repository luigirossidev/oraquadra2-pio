// OraQuadra V1.2.5  -  By Davide Gatti SURVIVAL HACKING  www.survivalhacking.it
// interfaccia web e alcuni effetti aggiunti da Marco Prunca
// Sketch completamente riscritto e ottimizzato per famiglia esp32 by Paolo Sambi
// Nuovo quadrante by Luca Beltramio
//
// Pulsante 1 = per lampeggio secondi sulla lettera E
// Pulsante 2 = per la cambio preset
// Pulsante 1 + Pulsante 2 premuti per 5 secondi = reset configurazione WiFi
// addon: setup ora automatica
//        effetto matrix
//        wifimanager ORAQUADRA_AP 192.168.4.1
//        ottimizzazione creazione parole per ora e minuti
//        interfaccia web per controllo completo
// 
// V1.0.1 - 21/02/2025
// Rimappatura nuovo quadrante e aggiunta gestione nuove WORD
// Sistemazione troncature minuti quando è il primo minuto della decade   21 = VENT UN / 31 = TRENT UN   
//
// V1.0.2 - 20/05/2025
// aggiunta la possibilità di usare pulsanti normali oltre a quelli touch
// BUTTON_LOGIC_INVERTED = 1 per pulsanti normali NA verso GND
// BUTTON_LOGIC_INVERTED = 0 per pulsanti touch
//
// V1.1.0 - 1/11/2025 by Marco Prunca
// aggiunto effetto MOTO con scia arcobaleno 
// MODIFICATO: la scia colora le scritte dell'orario man mano che passa
// aggiunto effetto GALAGA - sparo dal basso che colpisce le lettere
//
// V1.2.1 - 20/11/2025 - By Marco Prunca
// aggiunto effetto PACMAN - Pacman che insegue fantasmi colorando le scritte
// aggiunta interfaccia web per il controllo completo
//
// V1.2.2 - 22/11/2025 - By Davide Gatti
// aggiunto effetto DIGITALE - orologio digitale su schermo oraquadra
// aggiunta modalità giorno/sera/notte per la regolazione della luminosità su tre fasce orarie
// aggiunte scritte scorrevoli per informare l'utente sullo stato della connettività WIFI
// sistemati alcuni bug relativi a vari effetti e memorizzazione parametri di configurazione
// Sitemato grazie a Alessndro Spagnoletti e Paolo Sambinello l'effetto matrix che ora è corretto !!!!
//
// V1.2.3 - 27/11/2025 - By Marco Prunca
// Sistemati vari bug che facevano funzionare male la parte WEB
//
// V1.2.4 - 02/12/2025 - By Leotech e Paolo Sambinello
// Aggiunta funzione Testo scorrevole da web server
// Aggiunta possibilità di attivare/disattivare il quadrante digitale che si attiva ogni 6 sec per rendere visibile l'ora per postazioni più lontane. 
// L'attivazione/disattivazione del quadrante digitale che si alterna agli effetti selezionati può avvenire o da web server o mediante la pressione prolungata (6 sec) di uno dei 2 pulsanti. 
// Aggiunta palette di colori avanzata per selezionare molteplici più colori del testo scorrevole o delle parole di alcuni effetti.
// Aggiunti possibilità di selezionare varie dimensioni di font
// Aggiunto effetto rainbow
// Sistemati e migliorati effetti goccie a tron
//
// V1.2.5 - 07/12/2025 - By Davide Gatti
// Gestione testo scorrevole come entità a parte e non come preset
// Aggiunta palette colori per il testo scorrevole
// Aggiunta modalità per mostrare ora in formato testuale
// Sistemata modalità notturna e luminosità giornaliera del testo scorrevole.
//
// V1.2.6 - 10/12/2025 - By leotech
// Sistemato un problema che impediva la memorizzazione della velocità di scorrimento del testo con la data
//
// V1.2.7 - 23/12/2025 - By Davide Gatti
// Sistemato un problema che a volte mostrava un ora sbagliata in quanto non conteggiava correttamente l'ora legale.
//
// V1.2.8 - 28/02/2026 - By Davide Gatti
// Sistemato un bug che in assenza di testo scorrevole e di orologio dgitale, non faceva commutare i vari effetti al cambio minuto
// Sistemato un bug nell'effto moto dove in orari pieni tipo 22:00/12:00 dove non ci sono minuti, rimaneva accesa la E
//
// Mappatura matrice
// S-015 O-014 N-013 O-012 U-011 L-010 E-009 Y-008 O-007 R-006 E-005 X-004 Z-003 E-002 R-001 O-000
// V-016 E-017 N-018 T-019 I-020 T-021 R-022 E-023 D-024 I-025 C-026 I-027 O-028 T-029 T-030 O-031
// E-047 C-046 Q-045 U-044 A-043 T-042 T-041 O-040 R-039 D-038 I-037 C-036 I-035 S-034 E-033 I-032
// N-048 I-049 U-050 N-051 D-052 I-053 C-054 I-055 Q-056 U-057 A-058 T-059 T-060 R-061 O-062 O-063
// T-079 N-078 I-077 J-076 V-075 E-074 N-073 T-072 U-071 N-070 O-069 D-068 I-067 E-066 C-065 I-064
// I-080 Q-081 N-082 S-083 E-084 D-085 I-086 C-087 I-088 A-089 S-090 S-091 E-092 T-093 T-094 E-095
// D-111 U-110 D-109 O-108 D-107 I-106 C-105 I-104 A-103 N-102 N-101 O-100 V-099 E-098 L-097 F-096
// U-112 E-113 I-114 H-115 E-116 L-117 P-118 Q-119 U-120 A-121 R-122 A-123 N-124 T-125 A-126 X-127
// E-143 R-142 C-141 K-140 U-139 V-138 E-137 N-136 T-135 I-134 T-133 R-132 E-131 N-130 T-129 A-128
// G-144 R-145 I-146 N-147 C-148 I-149 N-150 Q-151 U-152 A-153 N-154 T-155 A-156 U-157 N-158 O-159
// S-175 E-174 D-173 I-172 C-171 I-170 D-169 O-168 D-167 I-166 C-165 I-164 O-163 T-162 T-161 O-160
// D-176 I-177 E-178 C-179 I-180 Q-181 U-182 A-183 T-184 T-185 O-186 R-187 D-188 I-189 C-190 I-191
// Q-207 U-206 A-205 T-204 T-203 R-202 O-201 Q-200 U-199 I-198 N-197 D-196 I-195 C-194 I-193 O-192
// A-208 R-209 T-210 R-211 E-212 D-213 I-214 C-215 I-216 A-217 S-218 S-219 E-220 T-221 T-222 E-223
// U-239 N-238 D-237 I-236 C-235 I-234 A-233 N-232 N-231 O-230 V-229 E-228 O-227 S-226 E-225 I-224
// C-240 I-241 N-242 Q-243 U-244 E-245 D-246 U-247 E-248 U-249 M-250 I-251 N-252 U-253 T-254 I-255

#include "app.h"

void setup() {
   App::setup();
}

void loop() {
   App::loop();
}
