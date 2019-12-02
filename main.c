/**
 * @file main.c  Labo #6
 * @author François Archambault
 * @date   
 * @brief  
 *
 * @version 1.0
 * Environnement:
 *     Développement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.00)
 *     Matériel: Carte démo du Pickit3. PIC 18F45K20
  */

/****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include "conio.h"
#include "Lcd4Lignes.h"
#include "serie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/********************** CONSTANTES *******************************************/
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caractère cgram d'une tuile
#define MINE 2 //caractère cgram d'une mine




/********************** PROTOTYPES *******************************************/
void initialisation(void);
void lcd_init(void);
void lcd_gotoXY(unsigned char x, unsigned char y);
void lcd_ecritChar(unsigned char car);
void lcd_cacheCurseur(void);
void lcd_putMessage(const unsigned char *chaine);
void lcd_effaceAffichage(void);
void lcd_montreCurseur(void);
char getAnalog(char canal);
void lcd_montreCurseur(void);
void lcd_cacheCurseur(void);
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
void enleveTuilesAutour(char x, char y);
bool demine(char x, char y);

/****************** VARIABLES GLOBALES ****************************************/
const char accueil[] = {"Labo6 de Frank"}; //message d'accueil au début du jeu 
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caractères affichés au LCD
char m_tabMines[NB_LIGNE][NB_COL+1];//Tableau contenant les mines, les espaces et les chiffres

/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{   
    int nb = 10;
    int posX=10;
    int posY=2;
    initialisation();
    lcd_init();
    lcd_effaceAffichage();
    lcd_cacheCurseur();
    lcd_gotoXY(1,1);
    lcd_putMessage(accueil); //affiche message d'accueil
    __delay_ms(1000);
    lcd_effaceAffichage();
    initTabVue();
    rempliMines(nb);
    metToucheCombien(); 
      
    while(1)
    {
        lcd_montreCurseur();
        deplace(&posX,&posY);
        if(PORT_SW == 0)
        {
            if(demine(posX,posY) == true)
            {
                
            }
        }
        __delay_ms(100);
    }
}

/*
 * @brief Rempli le tableau m_tabVue avec le caractère spécial (définie en CGRAM
 *  du LCD) TUILE. Met un '\0' à la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    int i,j =0;
    
    for(i=0;i<NB_LIGNE;i++)
    {
        for(j=0;j<NB_COL;j++)
        {
            m_tabVue[i][j] = TUILE;
        }
        for(int g =0;g<NB_LIGNE;g++)
            {
                lcd_gotoXY(1,g+1);
                lcd_putMessage(m_tabVue[g]);
            } 
    }
}
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caractère MINE défini en CGRAM.
 * @param int nb, le nombre de mines à mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    int i=0;
    int j=0;
    int g=0;
    char endroitMine =0;
    int ligne =0;
    int colonne=0;
    
    for(i=0;i<NB_LIGNE;i++)
    {
        for(j=0;j<NB_COL;j++)
        {
            m_tabMines[i][j] = ' ';
        }
    }
    while(g != nb)
    {
        do
        {
            colonne =rand()%NB_COL;
            ligne = rand()%NB_LIGNE;
        }while(m_tabMines[ligne][colonne] != ' ');
        m_tabMines[ligne][colonne] = MINE; 
        g++;
    }
    
}
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche à 3 mines, alors la méthode place le code ascii de 3 dans
 * le tableau. Si la case ne touche à aucune mine, la méthode met le code
 * ascii d'un espace.
 * Cette méthode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    char nombre=0;
    for(int i=0;i<NB_LIGNE;i++)
    {
        for(int j=0;j<NB_COL;j++)
        {
            if(m_tabMines[i][j] != MINE)
            {
                nombre = (calculToucheCombien(i,j));
            }
            if(nombre != 0)
            {
                m_tabMines[i][j] = nombre+48;

            }
           nombre = 0;    
        }
    }
}

/*
 * @brief Calcul à combien de mines touche la case. Cette méthode est appelée par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a vérifier
 * @return char nombre. Le nombre de mines touchées par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    char nombre=0;
    if((ligne-1 || ligne+1 < NB_LIGNE) && (colonne-1 || colonne+1 < NB_COL) )
    {
    
        if(m_tabMines[ligne-1][colonne-1]== MINE)
        {
            nombre = nombre+1;
        }
        if(m_tabMines[ligne-1][colonne]== MINE)
        {
            nombre = nombre+1;
        }
        if(m_tabMines[ligne-1][colonne+1]== MINE)
        {
            nombre = nombre+1;
        } 
        if(m_tabMines[ligne][colonne-1]== MINE)
        {
            nombre = nombre+1;
        }    
        if(m_tabMines[ligne][colonne+1]== MINE)
        {
            nombre = nombre+1;
        }    
        if(m_tabMines[ligne+1][colonne-1]== MINE)
        {
            nombre = nombre+1;
        }   
        if(m_tabMines[ligne+1][colonne]== MINE)
        {
            nombre = nombre+1;
        }    
        if(m_tabMines[ligne+1][colonne+1]== MINE)
        {
            nombre = nombre+1;
        }
    }
    return nombre;
}
/**
 * @brief Si la manette est vers la droite ou la gauche, on déplace le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    if(getAnalog(AXE_X) > 220)
    {
        (*x) = (*x) +1;
        lcd_gotoXY(((*x)),(*y));
        
        if((*x) > 20) //si *x est plus grand que 20(qui est la colonne 20), envoyer le vaisseau à la colonne 1
        {
            (*x)=1;
        }        
    }
    if(getAnalog(AXE_X) < 30)
    {
        (*x) = (*x) -1;
        lcd_gotoXY(((*x)),(*y));
         
        if((*x) < 1)//si *x est plus petit que 1 (qui est la colonne 1), envoyer le vaisseau à la colonne 20
        {
            (*x)=20;
        }        
    }
    if(getAnalog(AXE_Y) > 220)
    {
        (*y) = (*y) +1;
        lcd_gotoXY(((*x)),(*y));
         
        if(*y > 4)
        {
            *y=1;
        }          
        
    }
    if(getAnalog(AXE_Y) < 30)
    {
        (*y) = (*y) -1; 
        lcd_gotoXY(((*x)),(*y));
        
        if(*y < 1)
        {
            *y=4;
        }         
        
    }
}
/*
 * @brief Dévoile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derrière les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    lcd_gotoXY(x,y);
    lcd_ecritChar(m_tabMines[y][x]);
    
}
 
/*
 * @brief Dévoile les cases non minées autour de la tuile reçue en paramètre.
 * Cette méthode est appelée par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    
}
/*
 * @brief Lit le port analogique. 
 * @param Le no du port à lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal)
{ 
    ADCON0bits.CHS = canal;
    __delay_us(1);  
    ADCON0bits.GO_DONE = 1;  //lance une conversion
    while (ADCON0bits.GO_DONE == 1) //attend fin de la conversion
        ;
    return  ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}
/*
 * @brief Fait l'initialisation des différents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 à RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN à on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement à gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB à gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fréquence pour la conversion la plus longue possible)
 
}
