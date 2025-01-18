# Real-time Physical Material Aging and Weathering Simulation

A real-time graphics application demonstrating advanced material aging and weathering effects using OpenGL compute shaders. This project simulates realistic material degradation, rust formation, and environmental effects with physically-based rendering.

## Features

### Material Aging System
- Dynamic rust formation with multi-scale detail
- Progressive paint deterioration and cracking
- Realistic weathering patterns based on environmental exposure
- Physical-based material property evolution

### Environmental Effects
- Dynamic weather system affecting material aging
- Real-time puddle formation and ripples
- Cloud coverage and lighting variations
- Surface moisture accumulation and drying

### Physics Integration
- Real-time collision detection and response
- Verlet integration for object motion
- Environmental interaction affecting aging patterns
- Dynamic object behavior with realistic physics

### Rendering Technology
- Real-time ray tracing using compute shaders
- Physically-based rendering (PBR) material system
- Dynamic lighting adaptation to weather conditions
- Interactive parameter control

## Controls

- **WASD**: Movement
- **Mouse**: Look around
- **Space**: Jump
- **R/F**: Adjust rust level and age
- **M/N**: Control moisture level

## Technical Details

### Requirements
- OpenGL 4.3+
- C++17 compatible compiler
- CMake 3.10+
- Dependencies:
  - GLFW
  - GLM
  - stb_image

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Project Structure

```
aging_shader/
├── src/
│   ├── core/           # Core systems implementation
│   └── shaders/        # GLSL shader files
│       ├── common/     # Common shader utilities
│       ├── intersect/  # Ray intersection code
│       └── materials/  # Material definitions
└── textures/          # Texture assets
```

### Implementation Highlights

#### Rust Formation
The rust simulation uses a multi-scale noise approach:
- Large-scale pattern: Base rust distribution
- Medium-scale: Detail and variation
- Micro-scale: Surface roughness
- Edge weathering detection
- Moisture influence on rust formation

#### Paint Aging
Progressive deterioration including:
- Color fading and yellowing
- Crack formation
- Paint peeling
- Normal map generation
- Surface property evolution

#### Ground System
- Procedural crater formation
- Dynamic puddle system
- Weather-responsive surface properties
- Real-time ripple effects

## Performance Considerations

- Optimized compute shader dispatch
- Efficient ray-object intersection
- Cached material calculations
- Dynamic level of detail based on distance

## Known Issues

- High GPU memory usage with multiple objects
- Performance impact with excessive weather particles
- Some edge cases in physics collision detection

## Future Improvements

- Additional material aging types
- Enhanced weather particle systems
- More complex object interactions
- Extended material property controls
- Improved performance optimization

## Credits

Developed by Shaoxuan Yin as part of the TNM084 course at Linköping University.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
