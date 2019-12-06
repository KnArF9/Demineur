/**
 * @file main.c  Labo #6
 * @author Fran�ois Archambault
 * @date   5 d�cembre 2019
 * @brief  Le programme est un jeu de d�mineur. Le jeu commence et les cases du LCD sont toutes cach�es. Le premier tour, il y a 5 mines et le but
 * du jeu est de d�miner toutes les mines. Pour d�miner les mines, il faut se d�placer sur le LCD avec la manette. Pour d�miner une case, il faut appuyer
 * sur le bouton de la manette. Si la case d�min�e est un espace vide, les 8 cases autours de la case seront d�min�es. Si la case d�min�e est un chiffre,
 * �a indique au joueur combien de mines la case touche. Si c'est une mine, la partie est fini et toutes les tuiles sont d�voil�es. Si toutes les cases
 * sans mine sont d�min�es, le joueur gagne la partie et pour recommencer � jouer, il doit appuyer sur le bouton et la partie va recommencer avec 
 * une mine de plus dans le LCD.  
 * @version 1.0
 * Environnement:
 *     D�veloppement: MPLAB X IDE (version 5.05)
 *     Compilateur: XC8 (version 2.00)
 *     Mat�riel: Carte d�mo du Pickit3. PIC 18F45K20
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
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine




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
void afficheTabVue(void);
void afficheTabMine(void);
bool gagne(int* pMines);

/****************** VARIABLES GLOBALES ****************************************/
const char accueil[] = {"Labo6 de Frank"}; //message d'accueil au d�but du jeu 
char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caract�res affich�s au LCD
char m_tabMines[NB_LIGNE][NB_COL+1];//Tableau contenant les mines, les espaces et les chiffres

/*               ***** PROGRAMME PRINCPAL *****                             */
void main(void)
{   
    int nb = 5;
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
        lcd_gotoXY(posX,posY);
        lcd_montreCurseur(); // montre le curseur � la position de posX et posY
        deplace(&posX,&posY); //appel � la m�thode deplace avec les adresses de posX et posY
        if(PORT_SW == 0) //si le bouton est appuy�
        {
            while(PORT_SW == 0); //tant que le bouton est appuy�, reste bloqu�
            demine(posX,posY); //appel � la m�thode demine avec poX et posY
            if(demine(posX,posY) == false || gagne(&nb)== true) //si le retour de demine est faux ou si la m�thode gagne est �gal � vrai
            {
                while(PORT_SW != 0) //tant que le bouton n'est pas appuy�
                {
                    afficheTabMine(); //appel la m�thode pour afficher le m_tabMines
                }
                lcd_effaceAffichage(); //recommencer une nouvelle partie avec les param�tres de d�part (quand perdu)
                initTabVue();
                rempliMines(nb);
                metToucheCombien();
            }
        }
        
        __delay_ms(100); //d�lai pour g�rer le mouvement du curseur
    }
}

/*
 * @brief Rempli le tableau m_tabVue avec le caract�re sp�cial (d�finie en CGRAM
 *  du LCD) TUILE. Met un '\0' � la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    int i,j =0;
    
    for(i=0;i<NB_LIGNE;i++) //compteur qui passe toutes les cases du tableau m_tabVue pour le remplir de Tuile
    {
        for(j=0;j<NB_COL;j++)
        {
            m_tabVue[i][j] = TUILE;
        }
        for(int g =0;g<NB_LIGNE;g++)
            {
                
                lcd_putMessage(m_tabVue[g]); //affiche sur le LCD
            } 
    }
}
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caract�re MINE d�fini en CGRAM.
 * @param int nb, le nombre de mines � mettre dans le tableau 
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
    
    for(i=0;i<NB_LIGNE;i++) //compteur qui passe toutes les cases du tableau m_tabMines pour le vider avec des ' ' 
    {
        for(j=0;j<NB_COL;j++)
        {
            m_tabMines[i][j] = ' ';
        }
    }
    while(g != nb) // tant que g est diff�rent du nombre de mines
    {
        do
        {
            colonne =rand()%NB_COL; //colonne al�atoire
            ligne = rand()%NB_LIGNE;//ligne al�atoire
        }while(m_tabMines[ligne][colonne] != ' '); //v�rification pour voir s'il n'y a pas d�j� une mine � cet endroit  
        m_tabMines[ligne][colonne] = MINE; //met un mine � l'endroit al�atoire
        g++; 
    }
    
}

/*
 * @brief V�rifie si gagn�. On a gagn� quand le nombre de tuiles non d�voil�es
 * est �gal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagn�.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagn�, faux sinon
 */
bool gagne(int* pMines)
{
    int nbrTuile =0;
    for(int i=0;i<NB_LIGNE;i++) //compteur qui passe toutes les cases du tableau m_tabVue
    {
        for(int j=0;j<NB_COL;j++)
        {
            if(m_tabVue[i][j] == TUILE) //condition pour voir si la case point�e par le compteur est une tuile
            {
                nbrTuile++; //si la condition est vrai, nbrTuile +1
            }
        }
    }
    if(nbrTuile == (*pMines)) //si le nombre de tuile restante est �gal au nombre de mines, le joueur a gagn�
    {
        (*pMines)= (*pMines)+1; //augmentation du nombre de mines pour la prochaine partie
        return true;
    }
    else
    {
        return false;
    }
}
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche � 3 mines, alors la m�thode place le code ascii de 3 dans
 * le tableau. Si la case ne touche � aucune mine, la m�thode met le code
 * ascii d'un espace.
 * Cette m�thode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    char nombre=0;
    for(int i=0;i<NB_LIGNE;i++)//compteur qui passe toutes les cases du tableau m_tabMines
    {
        for(int j=0;j<NB_COL;j++)
        {
            if(m_tabMines[i][j] != MINE) //si la case point�e par le compteur n'est pas une mine
            {
                nombre = (calculToucheCombien(i,j)); //appel � la m�thode qui calcule le chiffre qu'il y aura � cet endroit. R�sultat plac� dans nombre
            }
            if(nombre != 0) //si nombre n'est pas �gal � 0
            {
                m_tabMines[i][j] = nombre+48; //48 = 0 dans la table ascii 49=1... 

            }
           nombre = 0;    
        }
    }
}

/*
 * @brief Calcul � combien de mines touche la case. Cette m�thode est appel�e par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    char nombre=0;
    if((ligne-1 >= NB_LIGNE || ligne+1 <= NB_LIGNE) && (colonne-1 >= NB_COL || colonne+1 <= NB_COL) ) //condition de limite
    {
    
        if(m_tabMines[ligne-1][colonne-1]== MINE) //conditions pour savoir si la case a une ou plusieurs mines autours d'elle
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
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    lcd_montreCurseur();
    if(getAnalog(AXE_X) > 220)//si la manette est vers la droite sur l'axe des X
    {
        (*x) = (*x) +1; //valeur de *x +1 pour aller vers la droite
        lcd_gotoXY(((*x)),(*y)); //pour que le curseur soit plac� au bon endroit
        
        if((*x) > 20) //si *x est plus grand que 20(qui est la colonne 20), envoyer le curseur � la colonne 1
        {
            (*x)=1;
        }        
    }
    if(getAnalog(AXE_X) < 30)//si la manette est vers la gauche sur l'axe des X
    {
        (*x) = (*x) -1;//valeur de *x -1 pour aller vers la gauche
        lcd_gotoXY(((*x)),(*y));//pour que le curseur soit plac� au bon endroit
         
        if((*x) < 1)//si *x est plus petit que 1 (qui est la colonne 1), envoyer le curseur � la colonne 20
        {
            (*x)=20;
        }        
    }
    if(getAnalog(AXE_Y) > 220)//si la manette est vers le bas sur l'axe des Y
    {
        (*y) = (*y) +1;//valeur de *y +1 pour aller vers le bas
        lcd_gotoXY(((*x)),(*y));//pour que le curseur soit plac� au bon endroit
         
        if(*y > 4)//si *y est plus grand que 4 (ligne 4), envoyer le curseur � la ligne 1
        {
            *y=1;
        }          
        
    }
    if(getAnalog(AXE_Y) < 30)//si la manette est vers le haut sur l'axe des Y
    {
        (*y) = (*y) -1; //valeur de *y pour aller vers le haut
        lcd_gotoXY(((*x)),(*y));//pour que le curseur soit plac� au bon endroit
        
        if(*y < 1)//si la valeur de *y est plus petite que 1 (colonne 1), envoyer le curseur sur la ligne 4
        {
            *y=4;
        }         
        
    }
}
/**
 * @brief affiche les lignes du tableau m_tabVue 
 * @param rien
 * @return rien
 */
void afficheTabVue(void)
{
    for(int i =0;i<NB_LIGNE;i++)//affiche les ligne du tableau m_tabVue
    {
        lcd_gotoXY(1,i+1);//i+1 car le tableau utilise de 0 � 3 et le LCD utilise 1 � 4
        lcd_putMessage(m_tabVue[i]);
    }
}

/**
 * @brief affiche les lignes du tableau m_tabMines 
 * @param rien
 * @return rien
 */
void afficheTabMine(void)
{
    for(int i =0;i<NB_LIGNE;i++)//affiche les ligne du tableau m_tabMines
    {
        lcd_gotoXY(1,i+1);//i+1 car le tableau utilise de 0 � 3 et le LCD utilise 1 � 4
        lcd_putMessage(m_tabMines[i]);
    }
}
/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    if(m_tabMines[y-1][x-1] == MINE) //si la case d�min�e est �gal � une mine. Le y et x sont -1 car le LCD est de 1 � 20 et le tableau est de 0 � 19
    {
        return false;// retourne faux et le joueur � perdu
    }
    if(m_tabMines[y-1][x-1] == 32) //si le case est �gal � un espace (32 = ascii pour espace)
    {
        enleveTuilesAutour(x-1,y-1); //appel � la fonction pour enlever 8 tuiles autours
        
    }
    else
    {
        m_tabVue[y-1][x-1] = m_tabMines[y-1][x-1];// copie de la case du tableau m_tabMines dans le tableau m_tabVue pour �tre affich�es
        
    }
    afficheTabVue(); //affiche m_tabVue
    return true;
}
 
/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    for(int i=-1;i<2;i++) //compteurs de 3 (i,j) pour faire le tour de la case d�min�e et qui est un espace 
    {
        for(int j=-1;j<2;j++)
        {
            m_tabVue[y+i][x+j] = m_tabMines[y+i][x+j];// copie des cases du tableau m_tabMines dans le tableau m_tabVue pour �tre affich�es
        }
    }
    
}
/*
 * @brief Lit le port analogique. 
 * @param Le no du port � lire
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
 * @brief Fait l'initialisation des diff�rents regesitres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
 
    ANSELH = 0;  // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
 
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
 
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
 
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
 
    ADCON0bits.ADON = 1; //Convertisseur AN � on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
 
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
 
}
