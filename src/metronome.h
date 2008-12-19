class Metronome : public fltk::Widget {

    int N, n;
    int last_beat;
    int plus;

    unsigned char r,g,b;

  public:

    Metronome(int x, int y, int w, int h, const char* label);

    int handle(int event);
    void draw();

    void update(int tick);
    void set_N(int zN);

};
