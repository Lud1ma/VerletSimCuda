#define WINDOW_X 1000
#define WINDOW_Y 800

#define CIRCLE_EDGES 64
#define SUB_STEPS 4
#define GRAVITY sf::Vector2f(0, 0.5)
#define FRICTION .999f
#define BOUNCE .9f
#define MARGIN 0.0f
#define HEAT_COEFFICENT 0.1

#include <iostream>
#include <boost/random.hpp>
#include <math.h>

#include <SFML/Graphics.hpp>

// MATHS
float distance(sf::Vector2f v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

sf::RenderWindow* window;

class Point {
private:
    sf::Vector2f position;
    sf::Vector2f old_position;
    float radius;
    float heat;
    
public:
    Point(sf::Vector2f pos, sf::Vector2f old_pos, float rad, float h) {
        position = pos;
        old_position = old_pos;
        radius = rad;
        heat = h;
    }
    float getRadius() {
        return radius;
    }
    sf::Vector2f getPosition() {
        return position;
    }
    float getHeat() {
        return heat;
    }
    void setPosition(sf::Vector2f p) {
        position = p;
    }
    void setHeat(float h) {
        heat = h;
    }
    void print() {
        std::cout << "Position: (" << position.x << "|" << position.y << "); \t Old Position: (" << old_position.x << "|" << old_position.y << ")" << std::endl;
    }
    void show() {
        draw(sf::Color(255, 0, 0));
    }
   
    void draw(sf::Color color) {
        sf::CircleShape circle = sf::CircleShape(radius, CIRCLE_EDGES); // Circle
        circle.setPosition(position-sf::Vector2f(radius, radius));
        circle.setFillColor(color);
        window->draw(circle);
    }
    void move() {
        sf::Vector2f d = position - old_position;
        old_position = position;
        position = position + (d + GRAVITY) * FRICTION;
    }
    void bound_edges() {
        sf::Vector2f v = position - old_position;
        if (position.x-radius < 0) {
            //print();
            position.x = radius+MARGIN;
            old_position.x = position.x + v.x*BOUNCE;
        } else if (position.x+radius > WINDOW_X) {
            //print();
            position.x = WINDOW_X-radius-MARGIN;
            old_position.x = position.x + v.x*BOUNCE;
        } 
        if (position.y-radius < 0) {
            //print();
            position.y = radius+MARGIN;
            old_position.y = position.y + v.y*BOUNCE;
        } else if (position.y+radius > WINDOW_Y) {
            //print();
            position.y = WINDOW_Y-radius-MARGIN;
            old_position.y = position.y + v.y*BOUNCE;
        }
    }
};

class Link {
private:
    Point* p1;
    Point* p2;
    float dist;
    
public:
    Link(Point* a, Point* b, float d) {
        p1 = a;
        p2 = b;
        dist = d;
    }
    
    void applyConstraint() {
        if (p1 != nullptr && p2 != nullptr) {
            sf::Vector2f direction = p2->getPosition() - p1->getPosition();
            float act_distance = distance(direction);

            if (act_distance < dist) {
                p1->setPosition(p1->getPosition()-((direction/act_distance)*((dist-act_distance)/2)));
                p2->setPosition(p2->getPosition()+((direction/act_distance)*((dist-act_distance)/2)));
            } else if (act_distance > dist) {
                p1->setPosition(p1->getPosition()-((direction/act_distance)*((dist-act_distance)/2)));
                p2->setPosition(p2->getPosition()+((direction/act_distance)*((dist-act_distance)/2)));
            }
        }
    }
};

class Obstacle {
private:
    sf::Vector2f position;
    float radius;
public:
    Obstacle (sf::Vector2f p, float r) {
        position = p;
        radius = r;
    }
    sf::Vector2f getPosition() {
        return position;
    }
    float getRadius() {
        return radius;
    }
    void draw() {
        sf::CircleShape circle = sf::CircleShape(radius, CIRCLE_EDGES);
        circle.setPosition(position-sf::Vector2f(radius, radius));
        circle.setFillColor(sf::Color::White);
        window->draw(circle);
    }
};

std::vector<Point*> points = std::vector<Point*>();
std::vector<Link*> links = std::vector<Link*>();
std::vector<Obstacle*> obstacles = std::vector<Obstacle*>();

void drawPoints() {
    for (Point* p: points) {
        p->show();
    }
    for (Obstacle* o: obstacles) {
        o->draw();
    }
}

void movePoints() {
    for (Point* p: points) {
        p->move();
    }
}

void bound_points() {
    for (int i = 0; i < points.size(); i++) {
        for (int j = 0; j < points.size(); j++) {
            if (i != j) {
                Point* p1 = points.at(i);
                Point* p2 = points.at(j);
                sf::Vector2f direction = p2->getPosition() - p1->getPosition();
                float min_distance = p1->getRadius() + p2->getRadius();
                float act_distance = distance(direction);
//                 float medium_heat = (p1->getHeat() + p2->getHeat()) / 2;
    
                if (act_distance < min_distance) {
                    p1->setPosition(p1->getPosition()-((direction/act_distance)*((min_distance-act_distance)/2)));
                    p2->setPosition(p2->getPosition()+((direction/act_distance)*((min_distance-act_distance)/2)));
//                     p1->setHeat((p1->getHeat() - medium_heat*HEAT_COEFFICENT));
//                     p2->setHeat((p2->getHeat() - medium_heat*HEAT_COEFFICENT));
                }
            }
        }
        for (Obstacle* o: obstacles) {
            Point* p1 = points.at(i);
            sf::Vector2f direction = o->getPosition() - p1->getPosition();
            float min_distance = p1->getRadius() + o->getRadius();
            float act_distance = distance(direction);
            
            if (act_distance < min_distance) {
                p1->setPosition(p1->getPosition()-((direction/act_distance)*((min_distance-act_distance))*BOUNCE));
            }
        }
    }
}

void applyAllConstraints() {
    for (int i = 0; i < SUB_STEPS; i++) {
        for (Point* p: points) {
            p->bound_edges();
        }
        bound_points();
        for (Link* l: links) {
            l->applyConstraint();
        }
    }
}


int main() {
    window = new sf::RenderWindow(sf::VideoMode(WINDOW_X, WINDOW_Y), "Verlet Simulator");
//     window->setFramerateLimit(60);
    int i = 0;
    
    points.push_back(new Point(sf::Vector2f(140, 80), sf::Vector2f(140, 80), 5, 50));
    points.push_back(new Point(sf::Vector2f(140, 70), sf::Vector2f(140, 70), 5, 50));
    points.push_back(new Point(sf::Vector2f(140, 60), sf::Vector2f(140, 60), 5, 50));
    points.push_back(new Point(sf::Vector2f(140, 50), sf::Vector2f(140, 50), 5, 50));
    points.push_back(new Point(sf::Vector2f(140, 45), sf::Vector2f(140, 45), 5, 50));
    points.push_back(new Point(sf::Vector2f(150, 40), sf::Vector2f(150, 40), 5, 50));
    points.push_back(new Point(sf::Vector2f(160, 40), sf::Vector2f(160, 40), 5, 50));
    points.push_back(new Point(sf::Vector2f(170, 45), sf::Vector2f(170, 45), 5, 50));
    points.push_back(new Point(sf::Vector2f(170, 50), sf::Vector2f(170, 50), 5, 50));
    points.push_back(new Point(sf::Vector2f(170, 60), sf::Vector2f(170, 60), 5, 50));
    points.push_back(new Point(sf::Vector2f(170, 70), sf::Vector2f(170, 70), 5, 50));
    points.push_back(new Point(sf::Vector2f(170, 80), sf::Vector2f(170, 80), 5, 50));
    
    links.push_back(new Link(points.at(0), points.at(1), 10));
    links.push_back(new Link(points.at(1), points.at(2), 10));
    links.push_back(new Link(points.at(2), points.at(3), 10));
    links.push_back(new Link(points.at(3), points.at(4), 10));
    links.push_back(new Link(points.at(4), points.at(5), 10));
    links.push_back(new Link(points.at(5), points.at(6), 10));
    links.push_back(new Link(points.at(6), points.at(7), 10));
    links.push_back(new Link(points.at(7), points.at(8), 10));
    links.push_back(new Link(points.at(8), points.at(9), 10));
    links.push_back(new Link(points.at(9), points.at(10), 10));
    links.push_back(new Link(points.at(10), points.at(11), 10));
    links.push_back(new Link(points.at(11), points.at(0), 10));
    
    obstacles.push_back(new Obstacle(sf::Vector2f(155, 50), 5));
//     obstacles.push_back(new Obstacle(sf::Vector2f(0, 800), 200));
//     obstacles.push_back(new Obstacle(sf::Vector2f(1000, 800), 200));
    
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window->close();
            }
        }
        
        if ((i % 2) == 0) {
            points.push_back(new Point(sf::Vector2f(500, 600), sf::Vector2f(515, 600), 2, 50));
            //std::cout << i/10 << std::endl;
        }
        
        window->clear(sf::Color::Black);
        
        movePoints();
        
        applyAllConstraints();
        
        drawPoints();
        
        window->display();
        
        int x = 0;
        for (Point* p: points) {
            if (p->getPosition().y > 1000) {
                x++;
            }
        }
        std::cout << points.size() << "\t" << x << std::endl;
        
        if (i < 501) i++;
    }
}



