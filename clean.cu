#define WINDOW_X 1000
#define WINDOW_Y 800

#define PARTICLES 5000
#define CIRCLE_EDGES 64
#define SUB_STEPS 8
#define GRAVITY_X 0.f
#define GRAVITY_Y .5f
#define FRICTION .999f
#define BOUNCE .9f
#define MARGIN 0.0f
#define HEAT_COEFFICENT 0.1

#include <iostream>
#include <boost/random.hpp>
#include <math.h>

#include <SFML/Graphics.hpp>

__global__ void move(float *position_x, float *position_y, float *old_position_x, float *old_position_y, float *radius, int N) {
    int tid = (blockIdx.x * blockDim.x) + threadIdx.x;
    
    if (tid > N) {
        return;
    }
    
    float d_x = position_x[tid] - old_position_x[tid];
    float d_y = position_y[tid] - old_position_y[tid];
    
    old_position_x[tid] = position_x[tid];
    old_position_y[tid] = position_y[tid];
    
    position_x[tid] = position_x[tid] + (d_x + GRAVITY_X) * FRICTION;
    position_y[tid] = position_y[tid] + (d_y + GRAVITY_Y) * FRICTION;
}

__global__ void boundaries(float *position_x, float *position_y, float *old_position_x, float *old_position_y, float *radius, int N) {
    int tid = (blockIdx.x * blockDim.x) + threadIdx.x;
    
    if (tid > N) {
        return;
    }
    
    float d_x = position_x[tid] - old_position_x[tid];
    float d_y = position_y[tid] - old_position_y[tid];
    
    if (position_x[tid] - radius[tid] < 0) {
        position_x[tid] = radius[tid];
        old_position_x[tid] = position_x[tid] +  d_x * BOUNCE;
    } else if (position_x[tid] + radius[tid] > WINDOW_X) {
        position_x[tid] = WINDOW_X - radius[tid];
        old_position_x[tid] = position_x[tid] +  d_x * BOUNCE;
    }
    
    if (position_y[tid] - radius[tid] < 0) {
        position_y[tid] = radius[tid];
        old_position_y[tid] = position_y[tid] +  d_y * BOUNCE;
    } else if (position_y[tid] + radius[tid] > WINDOW_Y) {
        position_y[tid] = WINDOW_Y - radius[tid];
        old_position_y[tid] = position_y[tid] +  d_y * BOUNCE;
    }
}

__global__ void collision(float *position_x, float *position_y, float *old_position_x, float *old_position_y, float *radius, int N) {
    int tid = (blockIdx.x * blockDim.x) + threadIdx.x;
 
    if (tid > N) {
        return;
    }
    
    for (int i = 0; i < N; i++) {
        if (tid != i) {
            float dir_x = position_x[tid] - position_x[i];
            float dir_y = position_y[tid] - position_y[i];
            float min = radius[i] + radius[tid];
            float dist = sqrt(dir_x * dir_x + dir_y * dir_y);
            
            if (dist <= min) {
                position_x[tid] = position_x[tid] + ((dir_x / dist) * ((min-dist) / 2));
                position_y[tid] = position_y[tid] + ((dir_y / dist) * ((min-dist) / 2));
                position_x[i] = position_x[i] - ((dir_x / dist) * ((min-dist) / 2));
                position_y[i] = position_y[i] - ((dir_y / dist) * ((min-dist) / 2));
                
//                 position_x[tid] = 0;
//                 position_x[i] = 0;
            }
        }
        
//         p1->setPosition(p1->getPosition()-((direction/act_distance)*((min_distance-act_distance)/2)));
//                     p2->setPosition(p2->getPosition()+((direction/act_distance)*((min_distance-act_distance)/2)));
    }
}

float *position_x, *position_y, *old_position_x, *old_position_y, *radius;
int ACTIVE_PARTICLES = 0;

void spawn(float pos_x, float pos_y, float old_pos_x, float old_pos_y, float rad) {
    position_x[ACTIVE_PARTICLES] = pos_x;
    position_y[ACTIVE_PARTICLES] = pos_y;   
    old_position_x[ACTIVE_PARTICLES] = old_pos_x;
    old_position_y[ACTIVE_PARTICLES] = old_pos_y;
    radius[ACTIVE_PARTICLES] = rad;
    ACTIVE_PARTICLES++;
}

int main() {
    size_t bytes = sizeof(float) * PARTICLES;

    
    cudaMallocManaged(&position_x, bytes);
    cudaMallocManaged(&position_y, bytes);
    cudaMallocManaged(&old_position_x, bytes);
    cudaMallocManaged(&old_position_y, bytes);
    cudaMallocManaged(&radius, bytes);
    
    int laps = 0;
    
    // NUMTHREADS
    int BLOCK_SIZE = 1 << 10;
    int GRID_SIZE = (PARTICLES + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Mainloop
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(WINDOW_X, WINDOW_Y), "Verlet Simulator CUDA");
    sf::CircleShape circle = sf::CircleShape(1., CIRCLE_EDGES); // Circle
    
    //window->setFramerateLimit(60);
    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
            }
        }
        
        window->clear(sf::Color::Black);
        
        float x, y;
        
        if (ACTIVE_PARTICLES < PARTICLES) {
            if (ACTIVE_PARTICLES < 500) {
                spawn(50.f, 700.f, 40.f, 700.f, rand() % 5);
            } else {
                if (laps % 2 == 0) {
                    x = rand() % WINDOW_X;
                    y = rand() % 400;
                    spawn(x, y, x, y, rand() % 5);
                }
            }
        } 
        
        for (int i = 0; i < PARTICLES; i++) {
            circle.setPosition(sf::Vector2f(position_x[i], position_y[i])-sf::Vector2f(radius[i], radius[i]));
            circle.setFillColor(sf::Color::Red);
            circle.setRadius(radius[i]);
            window->draw(circle);
        }
        
        move<<<GRID_SIZE, BLOCK_SIZE>>>(position_x, position_y, old_position_x, old_position_y, radius, ACTIVE_PARTICLES);
        cudaDeviceSynchronize();
        
        for (int i = 0; i < SUB_STEPS; i++) {
            boundaries<<<GRID_SIZE, BLOCK_SIZE>>>(position_x, position_y, old_position_x, old_position_y, radius, ACTIVE_PARTICLES);
            cudaDeviceSynchronize(); 
            collision<<<GRID_SIZE, BLOCK_SIZE>>>(position_x, position_y, old_position_x, old_position_y, radius, ACTIVE_PARTICLES);
            cudaDeviceSynchronize();
            //std::cout << position_x[1] - position_x[0] << "    " << position_y[1] - position_y[0] << "    " << radius[0] + radius[1]  << "    " << sqrt((position_x[1] - position_x[0]) * (position_x[1] - position_x[0]) + (position_y[1] - position_y[0]) * (position_y[1] - position_y[0])) << "    " << (sqrt((position_x[1] - position_x[0]) * (position_x[1] - position_x[0]) + (position_y[1] - position_y[0]) * (position_y[1] - position_y[0])) < radius[0] + radius[1]) << std::endl;;
        }
        
        window->display();
        laps++;
        std::cout << ACTIVE_PARTICLES << std::endl;
    }
    
    cudaFree(position_x);
    cudaFree(position_y);
    cudaFree(old_position_x);
    cudaFree(old_position_y);
    cudaFree(radius);
}
