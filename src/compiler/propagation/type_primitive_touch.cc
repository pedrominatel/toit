// Copyright (C) 2022 Toitware ApS.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; version
// 2.1 only.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// The license can be found in the file `LICENSE` in the top level
// directory of this repository.

#include "type_primitive.h"

namespace toit {
namespace compiler {

MODULE_TYPES(touch, MODULE_TOUCH)

TYPE_PRIMITIVE_ANY(init)
TYPE_PRIMITIVE_ANY(use)
TYPE_PRIMITIVE_ANY(unuse)
TYPE_PRIMITIVE_ANY(read)
TYPE_PRIMITIVE_ANY(get_threshold)
TYPE_PRIMITIVE_ANY(set_threshold)

}  // namespace toit::compiler
}  // namespace toit
