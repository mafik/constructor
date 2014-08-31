// Copyright 2014 Marek Rogalski <marek@mrogalski.eu>
#include <SDL_rwops.h>
#include <SkCanvas.h>
#include <SkPictureRecorder.h>
#include <cstdlib>
#include <string>
#define RAPIDXML_NO_EXCEPTIONS
#include "./rapidxml.hpp"
#include "./GL.h"
#include "./SVG.h"

#define DEBUG_SVG

void rapidxml::parse_error_handler(char const* msg, void*) {
  fprintf(stderr, "SVG parsing error: %s\n", msg);
  abort();
}

namespace constructor {
using std::string;
using rapidxml::xml_node;
using rapidxml::xml_document;
using rapidxml::xml_attribute;
typedef xml_node<char> node;
typedef xml_attribute<char> attr;

constexpr double p2m = 0.01 * 2.54 / 90;  // pixels to meters
constexpr double m2p = 1 / p2m;

static string ReadFile(const string& path) {
  SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
  if (rw != nullptr) {
    printf("File opened\n");
    Sint64 len = SDL_RWseek(rw, 0, RW_SEEK_END);
    SDL_RWseek(rw, 0, RW_SEEK_SET);
    printf("Size = %ld\n", len);
    char* buffer = new char[len];
    SDL_RWread(rw, buffer, len, 1);
    SDL_RWclose(rw);
    string s(buffer, len);
    delete[] buffer;
    return s;
  } else {
    fprintf(stderr, "Failed to open %s\n", path.c_str());
    return "";
  }
}

static double DoubleFromAttribute(const node& node, const string& name) {
  attr* a = node.first_attribute(name.c_str());
  if (a == 0) {
    fprintf(stderr, "Node %s doesn't have attribute %s!\n",
            node.name(), name.c_str());
    abort();
  }
  return atof(a->value());
}

static void DebugDraw(node* n) {
#ifdef DEBUG_SVG
  printf("Drawing <%s>\n", n->name());
  for (auto a = n->first_attribute(); a; a = a->next_attribute()) {
    printf("Found attribute \"%s\":\"%s\"\n", a->name(), a->value());
  }
  for (auto child = n->first_node(); child; child = child->next_sibling()) {
    printf("Found child <%s>\n", child->name());
  }
#endif
}

struct AutoTransform {
  SkCanvas* c;
  AutoTransform(node* n, SkCanvas* c) : c(c) {
    attr* a = n->first_attribute("transform");
    if (a == nullptr) {
      this->c = nullptr;
      return;
    }
    double m[6];
#ifdef DEBUG_SVG
    printf("Saving transform\n");
#endif
    c->save();
    string v = a->value();
    size_t i = 0;
    while (i < v.length()) {
      while (v[i] == ' ' && i < v.length()) ++i;
      if (!v.compare(i, 9, "translate")) {
        i += 9 + 1;  // skip opening paren
        char* end = const_cast<char*>(&v[i]);
        m[0] = strtod(end, &end);
        ++end;  // skip comma
        m[1] = strtod(end, &end);
        i = end - &v[0] + 1;  // skip closing paren
#ifdef DEBUG_SVG
        printf("Translating by %lf, %lf\n", m[0], m[1]);
#endif
        c->translate(m[0], m[1]);
      } else if (!v.compare(i, 6, "matrix")) {
        i += 6 + 1;  // skip opening paren
        char* end = const_cast<char*>(&v[i]);
        m[0] = strtod(end, &end);
        ++end;
        m[1] = strtod(end, &end);
        ++end;
        m[2] = strtod(end, &end);
        ++end;
        m[3] = strtod(end, &end);
        ++end;
        m[4] = strtod(end, &end);
        ++end;
        m[5] = strtod(end, &end);
        i = end - &v[0] + 1;  // skip closing paren
#ifdef DEBUG_SVG
        printf("Matrix transform: (%f, %f, %f)\n"
               "                  (%f, %f, %f)\n",
               m[0], m[2], m[4], m[1], m[3], m[5]);
#endif
        SkMatrix matrix;
        matrix.setAll(m[0], m[2], m[4], m[1], m[3], m[5], 0, 0, 1);
        c->concat(matrix);
      } else {
        i += 1;
        fprintf(stderr, "##Error!\n");
      }
    }
  }
  ~AutoTransform() {
    if (c) {
#ifdef DEBUG_SVG
      printf("Restoring transform\n");
#endif
      c->restore();
    }
  }
};

static void EatWhitespace(char** ptr) {
  while (**ptr && (**ptr == ' ' || **ptr == '\n' || **ptr == '\t')) ++*ptr;
}

static void EatSeparator(char** ptr, char sep) {
  EatWhitespace(ptr);
  if (**ptr == sep) {
    ++*ptr;
    EatWhitespace(ptr);
  }
}

static string ReadCSSName(char** ptr) {
  char* end = *ptr;
  while (*end && (islower(*end) || *end == '-')) ++end;
  string name(*ptr, end - *ptr);
  *ptr = end;
  return name;
}

static string ReadCSSValue(char** ptr) {
  char* end = *ptr;
  while (*end && (*end != ';')) ++end;
  string name(*ptr, end - *ptr);
  *ptr = end;
  return name;
}

static uint8_t ByteFromHexChar(char a) {
  if (a >= '0' && a <= '9') return a - '0';
  if (a >= 'A' && a <= 'F') return a - 'A' + 10;
  if (a >= 'a' && a <= 'f') return a - 'a' + 10;
  fprintf(stderr, "Hex char outside range: '%c'!", a);
  abort();
}

static uint8_t ByteFromHexChars(char a, char b) {
  return ByteFromHexChar(a) * 16 + ByteFromHexChar(b);
}

static SkColor ReadColor(const string& s) {
  if (s[0] == '#' && s.length() == 7) {
    uint8_t r = ByteFromHexChars(s[1], s[2]);
    uint8_t g = ByteFromHexChars(s[3], s[4]);
    uint8_t b = ByteFromHexChars(s[5], s[6]);
    return SkColorSetRGB(r, g, b);
  } else if (s == "none") {
    return SK_ColorTRANSPARENT;
  } else {
    fprintf(stderr, "Unknown color format: '%s'!", s.c_str());
    abort();
  }
}

static void ReadPaint(const string& s, const SkColor& currentColor,
                      SkPaint* paint) {
  if (s == "currentColor") {
    paint->setColor(currentColor);
#ifdef DEBUG_SVG
    printf("Setting color to current color\n");
#endif
  } else {
    SkColor fillColor = ReadColor(s);
    paint->setColor(fillColor);
  }
}

static void DrawPath(node* path, SkCanvas* canvas) {
  DebugDraw(path);
  attr* d = path->first_attribute("d");
  if (d == nullptr) return;
  AutoTransform t(path, canvas);
  char* c = d->value();
  char last_command = 0;
  char new_command = 0;
  double a[6];
  double pen[2] = { 0, 0 };
  SkPath sk_path;
  EatWhitespace(&c);
  while (*c) {
    new_command = 0;
    if (last_command && (isdigit(*c) || *c == '-')) {
      new_command = last_command;
    } else {
      new_command = *c;
      ++c;
      EatWhitespace(&c);
    }
    switch (new_command) {
      case 'm':
         pen[0] += strtod(c, &c);
         EatSeparator(&c, ',');
         pen[1] += strtod(c, &c);
         sk_path.moveTo(pen[0], pen[1]);
        break;
      case 'M':
         pen[0] = strtod(c, &c);
         EatSeparator(&c, ',');
         pen[1] = strtod(c, &c);
         sk_path.moveTo(pen[0], pen[1]);
        break;
      case 'l':
         pen[0] += strtod(c, &c);
         EatSeparator(&c, ',');
         pen[1] += strtod(c, &c);
         sk_path.lineTo(pen[0], pen[1]);
        break;
      case 'L':
         pen[0] = strtod(c, &c);
         EatSeparator(&c, ',');
         pen[1] = strtod(c, &c);
         sk_path.lineTo(pen[0], pen[1]);
        break;
      case 'z':
      case 'Z':
         sk_path.close();
        break;
      default:
        fprintf(stderr, "Unknown SVG path command: '%c'\n", new_command);
        abort();
    }
    last_command = new_command;
    if (last_command == 'm') last_command = 'l';
    if (last_command == 'M') last_command = 'L';
    EatWhitespace(&c);
  }
  SkColor currentColor = SK_ColorBLACK;
  SkPaint strokePaint;
  SkPaint fillPaint;
  bool invisible = false;
  fillPaint.setStyle(SkPaint::kFill_Style);
  fillPaint.setAntiAlias(true);
  strokePaint.setStyle(SkPaint::kStroke_Style);
  strokePaint.setAntiAlias(true);
  attr* style = path->first_attribute("style");
  c = style->value();
  EatWhitespace(&c);
  while (*c) {
    string name = ReadCSSName(&c);
    EatSeparator(&c, ':');
    string value = ReadCSSValue(&c);
    EatSeparator(&c, ';');
    printf("Style \"%s\" = \"%s\"\n", name.c_str(), value.c_str());
    if (name == "color") {
      currentColor = ReadColor(value);
    } else if (name == "fill-opacity") {
      double opacity = atof(value.c_str());
      fillPaint.setAlpha(static_cast<uint8_t>(opacity * 255));
    } else if (name == "fill-rule") {
      SkPath::FillType fill_type = SkPath::kWinding_FillType;
      if (value == "nonzero") {
        fill_type = SkPath::kWinding_FillType;
      } else if (value == "evenodd") {
        fill_type = SkPath::kEvenOdd_FillType;
      } else {
        fprintf(stderr, "Unknown fill type \"%s\"\n", value.c_str());
        abort();
      }
      sk_path.setFillType(fill_type);
    } else if (name == "fill") {
      ReadPaint(value, currentColor, &fillPaint);
    } else if (name == "stroke") {
      ReadPaint(value, currentColor, &strokePaint);
    } else if (name == "stroke-width") {
      double stroke_width = atof(value.c_str());
      if (value[value.length() - 1] == '%') {
        auto canvas_info = canvas->imageInfo().dimensions();
        auto w = canvas_info.width();
        auto h = canvas_info.height();
        stroke_width *= sqrt(w*w + h*h)/sqrt(2) / 100;
      }
      strokePaint.setStrokeWidth(stroke_width);
    } else if (name == "enable-background") {
      if (value != "accumulate") {
        fprintf(stderr, "Unknown background-filter specification: \"%s\"\n",
                value.c_str());
        abort();
      }
    } else if (name == "marker") {
      if (value != "none") {
        fprintf(stderr, "Path markers are not supported! \"%s\"\n",
                value.c_str());
        abort();
      }
    } else if (name == "visibility") {
      if (value == "visible") {
        invisible = false;
      } else if (value == "hidden" || value == "collapse") {
        invisible = true;
      } else {
        fprintf(stderr, "Unknown visibility: \"%s\"\n", value.c_str());
        abort();
      }
    } else if (name == "display") {
      if (value == "none") {
        invisible = true;
      }
    } else if (name == "overflow") {
      // ignore
    } else {
      fprintf(stderr, "Unknown style \"%s\" = \"%s\"\n",
              name.c_str(), value.c_str());
      abort();
    }
  }
  if (!invisible) {
    canvas->drawPath(sk_path, fillPaint);
    canvas->drawPath(sk_path, strokePaint);
  }
}

static void DrawG(node* g, SkCanvas* canvas) {
  DebugDraw(g);
  AutoTransform t(g, canvas);
  for (auto child = g->first_node(); child; child = child->next_sibling()) {
    string name = child->name();
    if (!name.compare("g")) {
      DrawG(child, canvas);
    } else if (!name.compare("path")) {
      DrawPath(child, canvas);
    }
  }
}

static SkPicture* DrawSVG(node* root) {
  DebugDraw(root);
  SkPictureRecorder recorder;
  double width = DoubleFromAttribute(*root, "width") * p2m;
  double height = DoubleFromAttribute(*root, "height") * p2m;
  SkCanvas* canvas = recorder.beginRecording(width, height);
  canvas->scale(p2m, p2m);
  for (auto child = root->first_node(); child; child = child->next_sibling()) {
    string name = child->name();
    if (!name.compare("g")) {
      DrawG(child, canvas);
    } else if (!name.compare("path")) {
      DrawPath(child, canvas);
    }
  }
  SkPaint paint;
  paint.setARGB(255, 233, 128, 192);
  paint.setAntiAlias(true);
  canvas->drawCircle(0, 0, 3.5, paint);
  canvas->flush();
  return recorder.endRecording();
}

SkPicture* LoadSVG(const string& resource_name) {
  string path("resources/" + resource_name + ".svg");
  string contents(ReadFile(path));
  printf("SVG \"%s\" read:\n%s\n", resource_name.c_str(), contents.c_str());
  xml_document<> doc;
  doc.parse<0>(const_cast<char*>(contents.c_str()));
  node* root = doc.first_node("svg", 3);
  if (root == 0) {
    fprintf(stderr, "SVG file \"%s\" doesn't contain root svg element!\n",
            path.c_str());
    return nullptr;
  }
  return DrawSVG(root);
}
}
