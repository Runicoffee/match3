#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <vector>
#include <deque>
#include <random>
#include <QTimer>
#include <QLabel>

/* fruit_flip
 *
 * Lyhyt Kuvaus:
 * Mainwindow -luokka tietorakenne. Ideana on luoda
 * Luo Match3 -kaltaisen peli. Peliin luodaan 10x10 ruudukko hedelmia
 * ja esteita. Vierekkaisten hedelmien paikkaa voi vaihtaa, jolloin
 * mahdolliset kolme vierekkaista hedelmaa poistetaan. Pelissa
 * on pisteiden lasku (poistetut hedelmat lkm) ja kello. Hedelmilla
 * on taustavari ja kuva. Hedelmat siirretaan ilmoittamalla kahden
 * hedelman koordinaatit ja painamalla Vaida nappia. Tarkempi kuvausta
 * instuctions.txt -tiedostossa
 *
 *
 * Huomioita:
 * Alkuperaiseen versioon on lisatty muun muassa
 * Fruit -struct, std::vector<QBrush> varit
 * ja useita yksityisia ja private slots -metodeja.
 *
 * */

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_line_edit_ensimmainen_textChanged(const QString &arg1);

    void on_line_edit_toinen_textChanged(const QString &arg1);

    void ajan_kasittely();

    void on_vaihda_hedelmat_clicked();

    void tiputa_hedelmat();

    void tarkasta_pelilauta();

    void on_check_box_ei_viivetta_clicked(bool);

private:
    Ui::MainWindow *ui;

    // Scene for the game grid
    QGraphicsScene* scene_;

    // Margins for the drawing area (the graphicsView object)
    // You can change the values as you wish
    const int TOP_MARGIN = 150;
    const int LEFT_MARGIN = 100;

    // Size of a square containing a fruit
    const int SQUARE_SIDE = 36; // give your own value here
    // Number of vertical cells (places for fruits)
    const int ROWS = 10; // give your own value here
    // Number of horizontal cells (places for fruits)
    const int COLUMNS = 10; // give your own value here

    // Constants describing scene coordinates
    const int BORDER_UP = 0;
    const int BORDER_DOWN = SQUARE_SIDE * ROWS;
    const int BORDER_LEFT = 0;
    const int BORDER_RIGHT = SQUARE_SIDE * COLUMNS;

    // Constants for different fruits and the number of them
    // Add/remove fruits as you wish, or you can remove the whole enum type
    // Actually only the value NUMBER_OF_FRUITS is needed in the
    // template code
    enum Fruit_kind {CHERRY,
                     BANANA,
                     EGGPLANT,
                     ORANGE,
                     PEAR,
                     APPLE,
                     EMPTY,
                     ROCK,
                     NUMBER_OF_FRUITS};
    //kunkin hedelman taustavari.
    std::vector<QBrush> varit = {Qt::magenta,
                                 Qt::red,
                                 Qt::green,
                                 Qt::yellow,
                                 Qt::blue,
                                 Qt::black,
                                 Qt::white,
                                 Qt::gray};

    // For randomly selecting fruits for the grid
    std::default_random_engine randomEng_;
    std::uniform_int_distribution<int> distr_;
    //Sisaltaa hedelman kaikki tarvittavat tiedot.
    //koordinaatit, hedelman tyypin, hedelma_itemin eli varin
    //ja hedelman label kuvan seka kuvan. struct Fruit tallennetaan
    //vektoriin, joka tallennetaan ruudukko vektoriin.
    struct Fruit {
        QPixmap kuva_;
        QLabel* hedelman_kuva_;
        QGraphicsRectItem* hedelma_item_;
        Fruit_kind hedelma_tyyppi_;
        int x_;
        int y_;
    };
    //kayttajan syotteet, joista saadaan siirrettavien
    //hedelmien koordinaatit
    std::string ensimmainen_;
    std::string toinen_;
    //kuvaa koko pelikenttaa, tallennetaan rivit jossa on struct
    //Fruit -hedelmat.
    std::vector<std::vector<Fruit>> ruudukko_;
    //Pelaajan saamat pisteet, eli poistettujen hedelmien
    //lukumaara.
    int pisteet_ = 0;
    //ajastin
    QTimer* timer;
    //sekunnit ja minuutit kuvaavat pelin kaynnistamisesta
    //kulunutta aikaa, eli peliaikaa.
    int sekunnit_ = 0;
    int minuutit_ = 0;

    int delay_ = 2000;

    // Writes the titles for the grid rows and columns
    void init_titles();

    //tarkistaa kayttajan koordinaatti syotteet
    bool syotteen_tarkastus();

    //siirtaa hedelmat keskenaan annettujen koordinaattien mukaan, jos mahdollista.
    void siirra_hedelmat(int ensimmainen_x, int ensimmainen_y, int toinen_x, int toinen_y);

    // Luo hedelmat ruudukolle pelin alussa.
    void init_ruudukko(); 

    // Draws a single fruit near the right bottom corner of the grid
    // At the moment, this function is not called in the template,
    // but try to do so, if you want to use real fruits instead of rectangles.
};
#endif // MAINWINDOW_HH

