/**
 * @file obstacle.hpp
 * @brief Defines an internal obstacle placeholder model for future extensions.
 */
#pragma once

#include <string>
#include <utility>
#include <variant>

namespace skymapf::world {

    /// Placeholder 2D box obstacle shape.
    struct Box2D {};
    /// Placeholder 3D box obstacle shape.
    struct Box3D {};

    /// Variant wrapper for supported obstacle shape placeholders.
    using ObstacleShape = std::variant<Box2D, Box3D>;

    /// Internal mutable obstacle record.
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