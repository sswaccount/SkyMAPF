#pragma once

namespace skymapf::world {

    using ObstacleShape = std::variant<Box2D, Box3D>;

    class Obstacle {
    public:
        Obstacle() = default;
    
        explicit Obstacle(std::string name, ObstacleShape shape)
            : name_(std::move(name)), shape_(std::move(shape)) {}
    
        const std::string& name() const noexcept { return name_; }
        void set_name(std::string name) { name_ = std::move(name); }
    
        const ObstacleShape& shape() const noexcept { return shape_; }
        void set_shape(ObstacleShape shape) { shape_ = std::move(shape); }
    
    private:
        std::string name_;
        ObstacleShape shape_;
    };

}