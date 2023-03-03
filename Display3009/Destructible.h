#ifndef DESTRUCTIBLE_H
#define DESTRUCTIBLE_H

class Destructible{
    public:
        Destructible();

        virtual void Destroy();
        void Mark();
        bool Marked();
    private:

        bool _destroyed;
};

#endif