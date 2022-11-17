#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QPixmap>
#include <map>
#include <cctype>

/* fruit_flip
 *
 * Lyhyt Kuvaus:
 * Sisaltaa mainwindow.hh:n funktot, konstrucktorin ja
 * destruktorin. Ideana luoda Match3 -kaltaisen pelin.
 * Peliin luodaan 10x10 ruudukko hedelmia
 * ja esteita mikali arpa niin suo.
 * Vierekkaisten hedelmien paikkaa voi vaihtaa, jolloin
 * mahdolliset kolme vierekkaista hedelmaa poistetaan. Pelissa
 * on pisteiden lasku (poistetut hedelmat lkm) ja kello. Hedelmilla
 * on taustavari ja kuva. Hedelmat siirretaan ilmoittamalla kahden
 * hedelman koordinaatit ja painamalla Vaida nappia. Tarkempi kuvaus pelista
 * instuctions.txt -tiedostossa
 *
 *
 *
 *
 * */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //luodaan ajastin pelin kelloa varten.
    timer = new QTimer(this);
    //yhdistetaan ajastimen timeout -signaali ajan_kasittelyyn.
    connect(timer, SIGNAL(timeout()), this, SLOT(ajan_kasittely()));
    //muokataan lcd_number -taulujen reunojen ja numeroiden varit
    //hauskoiksi.
    ui->lcd_number_sekunit->setPalette(Qt::green);
    ui->lcd_number_minuutit->setPalette(Qt::cyan);
    ui->lcd_number_pisteet->setPalette(Qt::red);
    //aloitetaan ajastin. timeout() signaali sekunnin (1000 millisekunnin)
    //valein.
    timer->start(1000);
    // We need a graphics scene in which to draw rectangles
    scene_ = new QGraphicsScene(this);
    // The width of the graphicsView is BORDER_RIGHT added by 2,
    // since the borders take one pixel on each side
    // (1 on the left, and 1 on the right).
    // Similarly, the height of the graphicsView is BORDER_DOWN added by 2.
    ui->graphicsView->setGeometry(LEFT_MARGIN, TOP_MARGIN,
                                  BORDER_RIGHT + 2, BORDER_DOWN + 2);
    ui->graphicsView->setScene(scene_);

    // The width of the scene_ is BORDER_RIGHT decreased by 1 and
    // the height of it is BORDER_DOWN decreased by 1, because
    // each square of a fruit is considered to be inside the sceneRect,
    // if its upper left corner is inside the sceneRect.
    scene_->setSceneRect(0, 0, BORDER_RIGHT - 1, BORDER_DOWN - 1);
    int seed = time(0); // You can change seed value for testing purposes
    randomEng_.seed(seed);
    //NUMBER_OF_FRUITS - 3, silla emme halua arpoa arvoa EMPTY, ROCK tai
    //NUMBER_OF_FRUITS.
    distr_ = std::uniform_int_distribution<int>(0, NUMBER_OF_FRUITS - 3);
    distr_(randomEng_); // Wiping out the first random number (which is almost always 0)
    //saadetaan line_edittien maksimikoko kahdeksi, silla koordinaatit ilmoitetaan
    //muotoa AA tai bb.
    ui->line_edit_ensimmainen->setMaxLength(2);
    ui->line_edit_toinen->setMaxLength(2);
    //ohjeviesti
    ui->text_browser_ohje->setText("Aseta koordinaatit tekstiruutuihin aakkosilla, "
                                   "ilman valimerkkeja, esimerkiksi AA tai bb.");
    //koordinaatit
    init_titles();
    //luodaan alkuruudukko
    init_ruudukko();
}
MainWindow::~MainWindow()
{
    delete timer;
    delete ui;
}
void MainWindow::init_titles()
{
    // Displaying column titles, starting from A
    for(int i = 0, letter = 'A'; i < COLUMNS; ++i, ++letter)
    {
        int shift = 5;
        QString letter_string = "";
        letter_string.append(letter);
        QLabel* label = new QLabel(letter_string, this);
        label->setGeometry(LEFT_MARGIN + shift + i * SQUARE_SIDE,
                           TOP_MARGIN - SQUARE_SIDE,
                           SQUARE_SIDE, SQUARE_SIDE);
    }

    // Displaying row titles, starting from A
    for(int i = 0, letter = 'A'; i < ROWS; ++i, ++letter)
    {
        QString letter_string = "";
        letter_string.append(letter);
        QLabel* label = new QLabel(letter_string, this);
        label->setGeometry(LEFT_MARGIN - SQUARE_SIDE,
                           TOP_MARGIN + i * SQUARE_SIDE,
                           SQUARE_SIDE, SQUARE_SIDE);
    }
}
//Luodaan alkuruudukko, missa on satunaisia hedelmia ja esteita. Hedelmia ja
//esteita on max 2 vierekkain.hedelmilla on seka omalle tyypille ominainen
//taustavari, etta kuva. Esteet esitetaan harmaana taustavarina, mutta mahd.
//tulevaisuutta varten niillekin maaritellaan kuva, kuvaa ei vain nayteta.
void MainWindow::init_ruudukko()
{
    int x_arvo = 0; //luotavan hedelman x_koordinaatti
    //luodaan sarake kerrallaan.
    for (int sarake = 0; sarake < COLUMNS; ++sarake) {
        std::vector<Fruit> y_akseli; //tallennetaan sarakkeen rivien hedelmat
        int y_arvo = 0; //luotavan hedelman y_koordinaatti.
        //kertoo, monennessako hedelmassa ruudukossa ollaan menossa.
        int y_akseli_indeksi = 0;
        //luodaan rivi
        for (int rivi = 0; rivi < ROWS; ++rivi) {
            int hedelman_numero = Fruit_kind(distr_(randomEng_));
            //kaksi samaa hedelmaa perakkain, lisataan este eli Fruit_kind ROCK.
            if (y_akseli.size() >= 2) {
                if (y_akseli.at(y_akseli.size()-1).
                        hedelma_tyyppi_ == hedelman_numero
                        and y_akseli.at(y_akseli.size()-2).
                        hedelma_tyyppi_ == hedelman_numero) {
                    hedelman_numero = ROCK;
                }
            }
            if (ruudukko_.size() >= 2) {
                if (ruudukko_.at(ruudukko_.size()-1).at(y_akseli_indeksi).
                        hedelma_tyyppi_ == hedelman_numero
                        and ruudukko_.at(ruudukko_.size()-2).at(y_akseli_indeksi).
                        hedelma_tyyppi_ == hedelman_numero) {
                    hedelman_numero = ROCK;
                }
            }
            //nyt luodaan itse hedelma ja lisataan ruudulle
            //kuva ja hedelman taustavari. Esteen tapauksessa kuvaa ei laiteta nakyviin.
            const std::vector<std::string>
                    fruits = {"pear", "bananas", "cherries", "orange", "eggplant","apple",
                              "strawberry", "tomato", "grapes"};
            // Defining where the images can be found and what kind of images they are
            const std::string PREFIX(":/");
            const std::string SUFFIX(".png");
            // Converting image (png) to a pixmap
            std::string filename = PREFIX + fruits.at(hedelman_numero) + SUFFIX;
            QPixmap image(QString::fromStdString(filename));
            // Scaling the pixmap
            image = image.scaled(SQUARE_SIDE - 2, SQUARE_SIDE - 2);
            // Setting the pixmap for a new label
            QLabel* label = new QLabel("", this);
            label->setGeometry (x_arvo + LEFT_MARGIN + sarake + 2,
                                y_arvo + TOP_MARGIN + rivi + 2, SQUARE_SIDE, SQUARE_SIDE);
            //Esteelle ei aseteta (ainakaan toistaiseksi) kuvaa silla sopivaa kuvaa ei ole.
            if (hedelman_numero != ROCK) {
                label->setPixmap(image);
            } //hedelman tausta -nelio
            QGraphicsRectItem* lisattava = scene_->addRect(x_arvo, y_arvo,
                                                           SQUARE_SIDE, SQUARE_SIDE);
            lisattava->setBrush(varit.at(hedelman_numero)); //lisataan taustavari
            lisattava->setX(sarake); //nelion x-arvo oikein.
            lisattava->setY(rivi); //nelion y-arvo oikein.
            //luodaan itse hedelma rakenne, joka lisataan ruudukkoon. Rakenne
            //sisaltaa hedelman kaikki tarvittavat tiedot.
            Fruit hedelma = {image, label, lisattava, Fruit_kind(hedelman_numero),
                             x_arvo, y_arvo};
            y_akseli.push_back(hedelma);//lisataan hedelma omaan kohtaan rivissa.
            y_arvo += 35; //seuraavan hedelman y_arvo.
            ++y_akseli_indeksi; //seuraavan hedelman y_indeksi.
        }
        ruudukko_.push_back(y_akseli);//lisataan sarakkeelle valmis rivi.
        x_arvo += 35; //seuraavan sarakkeen x_arvo
    }
}
//Tarkastetaan, onko kayttajan koordinaatti syotteet oikeanlaisia. Niiden taytyy
//olla max 2 kokoisia, ja sisaltaa vain haluttuja kirjaimia, jotka tosin saavat
//olla pienia tai isoja. jos syote on ok, pyritaan siirtamaan hedelmat.
//return: bool true tai false, kertoo oliko syote sallittu vai ei.
bool MainWindow::syotteen_tarkastus()
{
    bool ensimmainen_ok = false; //muutetaan jos ensimmainen syote ok.
    bool toinen_ok = false; //muutetaan jos toinen syote ok.
    //sallitut merkit ja niita vastaava koordinaatti luku.
    std::map<char, int> koordinaatit = {
        {'A', 0},
        {'B', 1},
        {'C', 2},
        {'D', 3},
        {'E', 4},
        {'F', 5},
        {'G', 6},
        {'H', 7},
        {'I', 8},
        {'J', 9}
    }; //tarkistetaan, ovatko ensimmaisen syotteen molemmat merkit sallittuja.
    if (ensimmainen_.size() == 2) {
        if (koordinaatit.find(toupper(ensimmainen_.at(1))) != koordinaatit.end()
                and koordinaatit.find(toupper(ensimmainen_.at(0)))
                != koordinaatit.end()){
            ensimmainen_ok = true;
        }
    } //tarkistetaan, ovatko toisen syotteen molemmat merkit sallittuja.
    if (toinen_.size() == 2) {
        if (koordinaatit.find(toupper(toinen_.at(1))) != koordinaatit.end()
                and koordinaatit.find(toupper(toinen_.at(0)))
                != koordinaatit.end()){
            toinen_ok = true;
        }
    } //molemmat ok, siirretaan_hedelmat syotetta vastaaviin koordinaatteihin.
    if (ensimmainen_ok and toinen_ok) {
        siirra_hedelmat(koordinaatit.at(toupper(ensimmainen_.at(0))),
                        koordinaatit.at(toupper(ensimmainen_.at(1))),
                        koordinaatit.at(toupper(toinen_.at(0))),
                        koordinaatit.at(toupper(toinen_.at(1))));
        return true; //palautetaan tosi, silla syote oli ok.
    } else {
        //virheviesti
        ui->text_browser_ohje->setText("Virhe koordinaateissa. Ilmoita"
                                       " koordinaatit isoilla tai pienilla"
                                       " aakkosilla, ilman muita merkkeja.");
        return false;
    }
}
//siirretaan hedelmat keskenaan. Tarkalleen vaihdetaan seka labelin kuva etta
//taustavari nelion vari. Vaihdetaan myos hedelman tyypit ja hedelman kuva.
//Kuitenkaan itse Qlabel tai QGraphicsitem ei ruudulla siirry, vaan niiden kuva
//ja vari muuttuuu. Fruit -structi ei myoskaan siirry vektorissa, vaan sen
//sisaltamia arvoja muutetaan. Tyhjan ruudun kanssa ei voi vaihtaa paikkaa.
//Esteen eli kiven kanssa ei myoskaan voi vaihtaa paikkaa.
//param int ensimmainen_x: ensimmaisen siirrettavan hedelman x-koordinaatti.
//param int ensimmainen_y: ensimmaisen siirrettavan hedelman y-koordinaatti.
//param int toinen_x: toisen siirrettavan hedelman x-koordinaatti.
//param int toinen_y: toisen siirrettavan hedelman y-koordinaatti.
void MainWindow::siirra_hedelmat(int ensimmainen_x, int ensimmainen_y,
                                 int toinen_x, int toinen_y)
{
    //tarkistetaan, etta vaihdettavat hedelmat ovat vierekkain, ja etta ne eivat
     //ole sama hedelma.
    if (((ensimmainen_x - toinen_x == 1) or (ensimmainen_x - toinen_x == -1) or
            (ensimmainen_x - toinen_x == 0)) and ((ensimmainen_y - toinen_y == 1) or
            (ensimmainen_y - toinen_y == -1) or (ensimmainen_y - toinen_y == 0)) and
            ((ensimmainen_x - toinen_x == 0) or (ensimmainen_y - toinen_y == 0))) {
        //maaritellaan hedelma ja sen tyyppi vaihtoa varten.
        Fruit eka_hedelma = ruudukko_.at(ensimmainen_x).at(ensimmainen_y);
        Fruit_kind eka_hedelma_tyyppi = eka_hedelma.hedelma_tyyppi_;
        Fruit toka_hedelma = ruudukko_.at(toinen_x).at(toinen_y);
        Fruit_kind toka_hedelma_tyyppi = toka_hedelma.hedelma_tyyppi_;
        //tarkistetaan, ettei kumpikaan siirrettavista ole este eli kivi tai tyhja ruutu.
        if (eka_hedelma_tyyppi != EMPTY and toka_hedelma_tyyppi != EMPTY and
                eka_hedelma_tyyppi != ROCK and toka_hedelma_tyyppi != ROCK and
                eka_hedelma_tyyppi != toka_hedelma_tyyppi) {
            //vaihdetaan hedelmien taustavarit keskenaan.
            ruudukko_.at(ensimmainen_x).at(ensimmainen_y).hedelma_item_
                    ->setBrush(varit.at(toka_hedelma_tyyppi));
            ruudukko_.at(toinen_x).at(toinen_y).hedelma_item_
                    ->setBrush(varit.at(eka_hedelma_tyyppi));
            //maaritellaan hedelmien kuvat vaihtoa varten
            QPixmap eka_kuva = ruudukko_.at(ensimmainen_x).at(ensimmainen_y).kuva_;
            QPixmap toka_kuva = ruudukko_.at(toinen_x).at(toinen_y).kuva_;
            //vaihdetaan hedelmien labelin kuvat keskenaan
            ruudukko_.at(ensimmainen_x).at(ensimmainen_y).hedelman_kuva_
                    ->setPixmap(toka_kuva);
            ruudukko_.at(toinen_x).at(toinen_y).hedelman_kuva_
                    ->setPixmap(eka_kuva);
            //vaihdetaa hedelmien kuvat keskenaan
            ruudukko_.at(ensimmainen_x).at(ensimmainen_y).kuva_ = toka_kuva;
            ruudukko_.at(toinen_x).at(toinen_y).kuva_ = eka_kuva;
            //vaihdetaan hedelmien tyypit keskenaan
            ruudukko_.at(ensimmainen_x).at(ensimmainen_y).hedelma_tyyppi_
                    = toka_hedelma_tyyppi;
            ruudukko_.at(toinen_x).at(toinen_y).hedelma_tyyppi_
                    = eka_hedelma_tyyppi;
        } else {
            ui->text_browser_ohje->setText("Voit vaihtaa paikkaa vain "
                                           "erilaisten hedelmien kesken!");
        }
    } else {
        ui->text_browser_ohje->setText("Hedelmien on oltava vierekkain vaaka -tai "
                                       " pystysuuntaisesti!");
    }
}
//tarkastetaan, sisaltaako pelilauta hedelmia, mita on samaa kolme vierekkain.
//mikali kolme vierekkaista hedelmaa loydetaan, poistetaan nama kolme hedelmaa.
//tilannetta, missa hedelmia voisi olla vierekkain enemman kuin kolme, ei oteta
//huomoon, vaan AINA POISTETAAN KOLME VIEREKKAISTA SAMAA hedelmaa. Poisto tapahtuu
//asettamalla hedelman arvoksi EMPTY, taustavarin valkoiseksi ja poistamalla
//hedelman kuvan nakyvista. Jos hedelmia poistettiin, tiputetaan hedelmat viiveella
//tiputa_hedelmat() -metodissa.
//tarkastetaan, sisaltaako pelilauta hedelmia, mita on samaa sorttia kolme vierekkain.
//mikali kolme vierekkaista hedelmaa loydetaan, poistetaan nama kolme hedelmaa.
//tilannetta, missa hedelmia voisi olla vierekkain enemman kuin kolme, ei oteta
//huomoon, vaan AINA POISTETAAN KOLME VIEREKKAISTA SAMAA hedelmaa. Poisto tapahtuu
//asettamalla hedelman arvoksi EMPTY, taustavarin valkoiseksi ja poistamalla hedelman
//kuvan nakyvista. Jos hedelmia poistettiin, tiputetaan hedelmat viiveella
//tiputa_hedelmat() -metodissa.
void MainWindow::tarkasta_pelilauta()
{
    bool hedelmia_poistettiin = false; //true, jos hedelmia kolme vierekkain
    //kaydaan kaikki hedelmat lapi
    for (int sarake = 0; sarake < COLUMNS; ++sarake){
        for (int rivi = 0; rivi < ROWS; ++rivi){
            //vektorin indeksien takia rivin oltava >= 2.
            if (rivi >= 2) {
                //jos kolme vierekkaista hedelmaa samoja, ja eivat ole EMPTY
                if (ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ ==
                    ruudukko_.at(sarake).at(rivi - 1).hedelma_tyyppi_ and
                    ruudukko_.at(sarake).at(rivi - 1).hedelma_tyyppi_ ==
                    ruudukko_.at(sarake).at(rivi - 2).hedelma_tyyppi_ and
                        ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ != EMPTY) {
                    hedelmia_poistettiin = true;
                    int laskin = 0;
                    //poistetaan kolme samaa hedelmaa.
                    while (laskin >= -2) {
                        ruudukko_.at(sarake).at(rivi + laskin).hedelma_tyyppi_ = EMPTY;
                        ruudukko_.at(sarake).at(rivi + laskin).hedelma_item_
                                ->setBrush(Qt::white);
                        ruudukko_.at(sarake).at(rivi+ laskin).hedelman_kuva_
                                ->clear();
                        pisteet_ += 1; //tuhottu hedelma = yksi piste.
                        --laskin;
                    }
                }
            }//vektorin indeksien takia sarakkeen oltava >= 2.
            if (sarake >= 2) {
                //jos kolme vierekkaista hedelmaa samoja, ja eivat ole EMPTY
                if (ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ ==
                    ruudukko_.at(sarake - 1).at(rivi).hedelma_tyyppi_ and
                    ruudukko_.at(sarake - 1).at(rivi).hedelma_tyyppi_==
                    ruudukko_.at(sarake - 2).at(rivi).hedelma_tyyppi_ and
                        ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ != EMPTY) {
                    hedelmia_poistettiin = true;
                    int laskin = 0;
                    //poistetaan kolme samaa hedelmaa.
                    while (laskin >= -2) {
                        ruudukko_.at(sarake + laskin).at(rivi).hedelma_tyyppi_ = EMPTY;
                        ruudukko_.at(sarake + laskin).at(rivi).hedelma_item_
                                ->setBrush(Qt::white);
                        ruudukko_.at(sarake+ laskin).at(rivi).hedelman_kuva_
                                ->clear();
                        pisteet_ += 1; //tuhottu hedelma = yksi piste.
                        --laskin;
                    }
                }
            }
        }
    }//Paivitetaan pisteet pelaajan nakyviin ja tiputetaan hedelmat.
    if (hedelmia_poistettiin) {
        ui->lcd_number_pisteet->display(pisteet_);
        QTimer::singleShot(delay_, this, SLOT(tiputa_hedelmat()));
    //tuhottavia hedelmia ei ollut, eli mahdollinen "ketjureaktio" on ohi.
    //Voidaan siis vapauttaa nappi takaisin kayttoon.
    } else {
        ui->vaihda_hedelmat->setEnabled(true);
        ui->check_box_ei_viivetta->setEnabled(true);

    }
}
//Tiputetaan hedelmat tyhjien ruutujen paalle. Luonnollisesti tyhjia ruutuja eika
//esteita tiputeta, vaan ne pysyvat paikoillaan. Kun kaikki hedelmat on tiputettu,
//on pelilauta syyta tarkastaa uudelleen uusien kolmen sarjojojen varalta.
//Jokaisen tiputuksen jalkeen tarkastetaan, voidaanko tiputtaa joku toinen
//hedelma (esim tiputetun hedelman paalla oleva hedelma)
void MainWindow::tiputa_hedelmat()
{
    int laskin = 0; //kasvatetaan, kun kaikki hedelmat on kayty lapi.
    int tiputus_kerrat = 1; //kasvatetaan aina jokaisen tiputuksen jalkeen.
    //niin kauan kun voi olla uutta tiputettavaa, kaydaan hedelmia lapi.
    while (laskin < tiputus_kerrat) {
        //kaydaan kaikki hedelmat lapi
        for (int sarake = 0; sarake < COLUMNS; ++sarake){
            for (int rivi = 0; rivi < ROWS; ++rivi){
                if (rivi > 0) { //turha katsoa rivia mihin ei voi tiputtaa.
                    //jos on tyhja ja sen paalla on hedelma (ei este), voidaan tiputtaa.
                    if (ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ == EMPTY and
                            ruudukko_.at(sarake).at(rivi - 1).hedelma_tyyppi_ != EMPTY and
                            ruudukko_.at(sarake).at(rivi - 1).hedelma_tyyppi_ != ROCK){
                        //maaritetaan hedelmien tyypit vaihtoa varten
                        Fruit eka_hedelma = ruudukko_.at(sarake).at(rivi);
                        Fruit_kind eka_hedelma_tyyppi = eka_hedelma.hedelma_tyyppi_;
                        Fruit toka_hedelma = ruudukko_.at(sarake).at(rivi-1);
                        Fruit_kind toka_hedelma_tyyppi = toka_hedelma.hedelma_tyyppi_;
                        //maaritellaan kuvat vaihtoa varten
                        QPixmap toka_kuva = toka_hedelma.kuva_;
                        QPixmap eka_kuva = eka_hedelma.kuva_;
                        //vaihdetaan hedelmien taustavarit keskenaan.
                        ruudukko_.at(sarake).at(rivi).hedelma_item_
                                ->setBrush(varit.at(toka_hedelma_tyyppi));
                        ruudukko_.at(sarake).at(rivi - 1).hedelma_item_
                                ->setBrush(varit.at(eka_hedelma_tyyppi));
                        //vaihdetaan kuvat, uuteen tyhjaan kohtaan toki poistetaan
                        //kuva nakyvista.
                        ruudukko_.at(sarake).at(rivi).hedelman_kuva_
                                ->setPixmap(toka_hedelma.kuva_);
                        ruudukko_.at(sarake).at(rivi - 1).hedelman_kuva_->clear();
                        //vaihdetaan kuvat keskenaan
                        ruudukko_.at(sarake).at(rivi).kuva_ = toka_kuva;
                        ruudukko_.at(sarake).at(rivi - 1).kuva_ = eka_kuva;
                        //vaihdetaan hedelmien tyypit keskenaan.
                        ruudukko_.at(sarake).at(rivi).hedelma_tyyppi_ = toka_hedelma_tyyppi;
                        ruudukko_.at(sarake).at(rivi - 1).hedelma_tyyppi_ = eka_hedelma_tyyppi;
                        //pyritaan tiputtamaan viela kerran varmuuden vuoksi.
                        ++tiputus_kerrat;
                    }
                }
            }
        }
        ++laskin; //kaikki kayty kerran lapi. laskin > tiputus_kerrat, kun on
        //tiputettu varmuuden vuoksi.
    }//tiputtamisen jalkeen on syyta tarkastaa pelilauta uudelleen.
    QTimer::singleShot(delay_, this, SLOT(tarkasta_pelilauta()));
}
//jos ensimmaiseen line_edittiin on laitettu tekstia, laitetaan se talteen.
void MainWindow::on_line_edit_ensimmainen_textChanged(const QString &arg1){
    ensimmainen_ = arg1.toStdString();
}
//jos toiseen line_edittiin on laitettu tekstia, laitetaan se talteen.
void MainWindow::on_line_edit_toinen_textChanged(const QString &arg1){
    toinen_ = arg1.toStdString();
}
//klikataan vaihda hedelmat nappia, tarkastetaan syotteet, kun syotteet ok
//estetaan vaihda napin kaytto ja tarkastetaan pelilauta, joka aiheuttaa
//hedelmien mahdollisen tiputtamisen ja uudelleen pelilaudan tarkastuksen.
//siivotaan line_editit klikkaamisen jalkeen, olivat syotteet ok tai ei.
void MainWindow::on_vaihda_hedelmat_clicked()
{
    //poistetaan mahdolliset virheviestit/ohjeviestit.
    ui->text_browser_ohje->clear();
    bool syotteet_ok = syotteen_tarkastus();
    if (syotteet_ok) {
        ui->vaihda_hedelmat->setDisabled(true);
        ui->check_box_ei_viivetta->setDisabled(true);
        tarkasta_pelilauta();
    }
    ui->line_edit_ensimmainen->clear();
    ui->line_edit_toinen->clear();
}
//Aina, kun ajastin kay, niin lisataan sekunteihin ja kun sekuntit 60,
//lisataan minuutteihin. Aika kuvaa pelin kaynnistemisesta kulunutta aikaa.
//sekunnit ja minuutit paivitetaan lcd_numer -tauluihin.
void MainWindow::ajan_kasittely()
{
    sekunnit_ += 1;
    if (sekunnit_ == 60) {
        minuutit_ += 1;
        sekunnit_ = 0;
    }
    ui->lcd_number_sekunit->display(sekunnit_);
    ui->lcd_number_minuutit->display(minuutit_);
}
//Viiveen, jolla hedelmat tippuu saa pois paalta halutessaan.
void MainWindow::on_check_box_ei_viivetta_clicked(bool checked)
{
    if (checked) {
        delay_ = 0;
    } else {
        delay_ = 2000;
    }
}
