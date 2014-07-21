#ifndef COLOR_H
#define COLOR_H

class Color
{
public:
    float r;
    float g;
    float b;

public:
    Color()
    {
    }

    Color(float r, float g, float b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    Color operator+(const Color &c) const
    { 
        return Color(r + c.r, g + c.g, b + c.b);
    }

    Color operator*(float factor) const
    {
        return Color(r * factor, g * factor, b * factor);
    }

    Color mult(const Color& c) const
    { 
        return Color(r * c.r, g * c.g, b * c.b);
    }

    void saturate() 
    {
        r = (r > 1.0f) ? 1.0f : r; 
        g = (g > 1.0f) ? 1.0f : g;
        b = (b > 1.0f) ? 1.0f : b;
    }

    static Color Red()   { return Color(1, 0, 0); }
    static Color Green() { return Color(0, 1, 0); }
    static Color Blue()  { return Color(0, 0, 1); }
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(1, 1, 1); }
};

#endif
