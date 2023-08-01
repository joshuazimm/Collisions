#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include <vector>


void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b) {
    int i = static_cast<int>(h / 60.0f) % 6;
    double f = (h / 60.0f) - static_cast<double>(i);
    double p = v * (1.0f - s);
    double q = v * (1.0f - f * s);
    double t = v * (1.0f - (1.0f - f) * s);

    switch (i) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
}

SDL_Color getColor(double angle) {
    double hue = angle;

    double r, g, b;
    HSVtoRGB(hue, 1.0f, 1.0f, r, g, b);

    Uint8 red = static_cast<Uint8>(r * 255);
    Uint8 green = static_cast<Uint8>(g * 255);
    Uint8 blue = static_cast<Uint8>(b * 255);
    Uint8 alpha = 255;

    SDL_Color color = { red, green, blue, alpha };
    return color;
}

struct Vector2 {
    double x, y;
    Vector2(double x = 0, double y = 0) : x(x), y(y) {}
    ~Vector2() {}

    double dot(Vector2 other) const {
        return x * other.x + y * other.y;
    }

    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(double scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    double length() const {
        return std::sqrt(x * x + y * y);
    }

    Vector2 normalized() const {
        double len = length();
        double invLength = 1.0f / len;
        if (len == 0) return Vector2();
        return Vector2(x * invLength, y * invLength);
    }
};

class Particle {
public:
    Particle(double x, double y, double vx, double vy, double ax, double ay, double radius, SDL_Color color)
    : pos(x, y), vel(vx, vy), acc(ax, ay), radius(radius), color(color) {}
    ~Particle() {}

    void update(double dt) {
        vel += acc * dt;
        pos += vel * dt;
    }

    void drawParticle(SDL_Renderer *renderer) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int i = 0; i < radius * 2; i++) {
            for (int j = 0; j < radius * 2; j++) {
                double dx = radius - i;
                double dy = radius - j;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, pos.x + dx, pos.y + dy);
                }
            }
        }
    }
    double radius;
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    SDL_Color color;
};

class Circle {
public:
    Circle(double x, double y, double radius) : pos(x, y), radius(radius) {}
    ~Circle() {}

    bool checkCollision(const Particle& particle) {
        double dist = (particle.pos - pos).dot(particle.pos - pos);
        return dist > (radius - particle.radius) * (radius - particle.radius);
    }

    void handleCollision(Particle& particle) {
        Vector2 collisionPoint = closestPointOnCircle(particle.pos);
        Vector2 normal = (particle.pos - collisionPoint).normalized();
        Vector2 reflection = particle.vel - (normal * 2.0f * particle.vel.dot(normal));
        particle.vel = reflection;

        // tunnel protection
        // particle.pos += normal * (particle.radius - (particle.pos - collisionPoint).length());
    }

    void drawCircle(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i <= radius * 2; i++) {
            for (int j = 0; j <= radius * 2; j++) {
                int dx = radius - i;
                int dy = radius - j;
                if (abs(dx * dx + dy * dy - radius * radius) < radius) {
                    SDL_RenderDrawPoint(renderer, pos.x + dx, pos.y + dy);
                }
            }
        }
    }

private:
    Vector2 pos;
    double radius;

    Vector2 closestPointOnCircle(const Vector2& point) {
        Vector2 direction = point - pos;
        direction = direction.normalized();
        return pos + direction * radius;
    }
};

std::vector<Particle> placeDotsInCircle(double radius, int numDots, Vector2 initPos) {
    std::vector<Particle> coordinates;
    double angle = 0;
    for (int i = 0; i < numDots; ++i) {
        SDL_Color color = getColor(angle);
        double x = radius * cos(angle * M_PI / 180.0f) + initPos.x;
        double y = radius * sin(angle * M_PI / 180.0f) + initPos.y;
        Particle circle(x, y, 0.0f, 0.0f, 0.0f, 2.0f, 6.0f, color);
        coordinates.push_back(circle);
        angle += 360.0 / numDots;
    }

    return coordinates;
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    bool quit = false;
    double counter = 0.0f, increment = 1.0f, buffer = 8.0f;
    SDL_Event event;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    window = SDL_CreateWindow("Collide", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2000, 1333, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    std::vector<Particle> particles = placeDotsInCircle(150.0f, 1000, Vector2(1000.0f, 667.0f));
    Circle perfectCircle(1000.0f, 667.0f, 600.0f);


    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        counter += increment;
        if (counter == increment * buffer) {
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        
        for (int i = 0; i < particles.size(); i++) {
            particles[i].update(0.005f);
            if (perfectCircle.checkCollision(particles[i])) {
                perfectCircle.handleCollision(particles[i]);
            }
            if (counter == increment * buffer) {
                particles[i].drawParticle(renderer);
            }
        }
        
        if (counter == increment * buffer) {
            perfectCircle.drawCircle(renderer);
            SDL_RenderPresent(renderer);
            counter = 0.0f;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}  