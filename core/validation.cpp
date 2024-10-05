#include "validation.hpp"

std::vector<const char*> Validation::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

bool Validation::enableValidationLayers = false;
