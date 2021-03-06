// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/* track-meta-data - Object holding meta data for a song.
 *
 * Copyright (C) 2012 Henner Zeller
 *
 * This file is part of GMediaRender.
 *
 * GMediaRender is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GMediaRender is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GMediaRender; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

// TODO: we're assuming that the namespaces are abbreviated with 'dc' and 'upnp'
// ... but if I understand that correctly, that doesn't need to be the case.

#include "track-meta-data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmldoc.h"

const char *TrackMetadata::TAG_TITLE    = "dc:title";
const char *TrackMetadata::TAG_ARTIST   = "upnp:artist";
const char *TrackMetadata::TAG_ALBUM    = "upnp:album";
const char *TrackMetadata::TAG_GENRE    = "upnp:genre";
const char *TrackMetadata::TAG_COMPOSER = "upnp:creator";

const std::string& TrackMetadata::get_field(TagLabel tag) const {
  static std::string kDefault;
  auto found = fields_.find(tag);
  return found != fields_.end() ? found->second : kDefault;
}

bool TrackMetadata::set_field(TagLabel tag, const std::string &value) {
  std::string &to_update = fields_[tag];
  if (to_update != value) {
    to_update = value;
    return true;
  } else {
    return false;
  }
}

bool TrackMetadata::ParseXML(const std::string &xml) {
  const auto doc = XMLDoc::Parse(xml);
  if (!doc) return false;

  const auto items = doc->FindChild("DIDL-Lite").FindChild("item");
  for (XMLElement elem : items.children()) {
    fields_[elem.name()] = elem.value();
  }
  return items.exists();
}

/*static*/ std::string TrackMetadata::DefaultCreateNewId() {
  // Generating a unique ID in case the players cache the content by
  // the item-ID. Right now this is experimental and not known to make
  // any difference - it seems that players just don't display changes
  // in the input stream. Grmbl.
  static unsigned int xml_id = 42;
  char unique_id[4 + 8 + 1];
  snprintf(unique_id, sizeof(unique_id), "gmr-%08x", xml_id++);
  return unique_id;
}

std::string TrackMetadata::GenerateDIDL(const std::string &id) const {
  XMLDoc doc;
  auto item = doc.AddElement("DIDL-Lite",
                             "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/")
    .SetAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/")
    .SetAttribute("xmlns:upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/")
    .AddElement("item").SetAttribute("id", id);
  for (auto pair : fields_) {
    item.AddElement(pair.first).SetValue(pair.second);
  }
  return doc.ToXMLString();
}

std::string TrackMetadata::ToXML(const std::string &original_xml,
                                 std::function<std::string()> idgen) const {
  auto doc = XMLDoc::Parse(original_xml);
  XMLElement items;
  if (!doc || !(items = doc->FindChild("DIDL-Lite").FindChild("item"))) {
    return GenerateDIDL(idgen ? idgen() : DefaultCreateNewId());
  }

  bool any_change = false;
  for (auto pair : fields_) {
    auto tag = items.FindChild(pair.first);
    if (tag.value() == pair.second) continue;
    if (!tag.exists()) tag = items.AddElement(pair.first);
    tag.SetValue(pair.second);
    any_change = true;
  }
  if (any_change) {
    // Only if we changed the content, we generate a new unique id.
    const std::string new_id = idgen ? idgen() : DefaultCreateNewId();
    doc->FindChild("DIDL-Lite").SetAttribute("id", new_id);
  }
  return doc->ToXMLString();
}
