/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include "PluginMetadata.h"

using namespace std;
using namespace Kompas::Utility;

namespace Kompas { namespace PluginManager {

PluginMetadata::PluginMetadata(const Configuration& conf) {
    /* Author(s), version */
    _authors = conf.values<string>("author");
    _version = conf.value<string>("version");

    /* Dependencies, replacements */
    _depends = conf.values<string>("depends");
    _replaces = conf.values<string>("replaces");

    const ConfigurationGroup* metadata = conf.group("metadata");
    translator.setFallback(metadata);
    translator.setPrimary(metadata, true);
    _name = translator.get("name");
    _description = translator.get("description");
}

}}
