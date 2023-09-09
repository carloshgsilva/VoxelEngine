#include "Stream.h"

#include <glm/vec2.hpp>
#include <vector>
#include <unordered_map>

struct TableHEAD {
    uint16_t majorVersion;
    uint16_t minorVersion;
    int32_t fontRevision; // TODO: Fixed = x / (1 << 16)
    uint32_t checksumAdjustment;
    uint32_t magicNumber;
    uint16_t flags;
    uint16_t unitsPerEm;
    uint64_t created;
    uint64_t modified;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    uint16_t macStyle;
    uint16_t lowestRecPPEM;
    int16_t fontDirectionHint;
    int16_t indexToLocFormat;
    int16_t glyphDataFormat;

    void Serialize(Stream& s) {
        s.Serialize(majorVersion);
        s.Serialize(minorVersion);
        s.Serialize(fontRevision);
        s.Serialize(checksumAdjustment);
        s.Serialize(magicNumber);
        s.Serialize(flags);
        s.Serialize(unitsPerEm);
        s.Serialize(created);
        s.Serialize(modified);
        s.Serialize(xMin);
        s.Serialize(yMin);
        s.Serialize(xMax);
        s.Serialize(yMax);
        s.Serialize(macStyle);
        s.Serialize(lowestRecPPEM);
        s.Serialize(fontDirectionHint);
        s.Serialize(indexToLocFormat);
        s.Serialize(glyphDataFormat);
    }
};

struct TableMAXP {
    int32_t version; // TODO: Fixed = x / (1 << 16)
    uint16 numGlyphs;
    uint16 maxPoints;
    uint16 maxContours;
    uint16 maxCompositePoints;
    uint16 maxCompositeContours;
    uint16 maxZones;
    uint16 maxTwilightPoints;
    uint16 maxStorage;
    uint16 maxFunctionDefs;
    uint16 maxInstructionDefs;
    uint16 maxStackElements;
    uint16 maxSizeOfInstructions;
    uint16 maxComponentElements;
    uint16 maxComponentDepth;

    void Serialize(Stream& s) {
        s.Serialize(version);
        s.Serialize(numGlyphs);
        s.Serialize(maxPoints);
        s.Serialize(maxContours);
        s.Serialize(maxCompositePoints);
        s.Serialize(maxCompositeContours);
        s.Serialize(maxZones);
        s.Serialize(maxTwilightPoints);
        s.Serialize(maxStorage);
        s.Serialize(maxFunctionDefs);
        s.Serialize(maxInstructionDefs);
        s.Serialize(maxStackElements);
        s.Serialize(maxSizeOfInstructions);
        s.Serialize(maxComponentElements);
        s.Serialize(maxComponentDepth);
    }
};

struct TTFFile {
    TableHEAD head;
    TableMAXP maxp;
    uint16_t numberOfHMetrics;
    std::vector<uint32_t> offsets;
    std::unordered_map<uint32_t, uint32_t> remap;

    struct Glyph {
        std::vector<glm::vec2> coords;
        glm::vec2 min;
        glm::vec2 max;
        float advanceWidth;
        float leftSideBearing;
    };
    std::vector<Glyph> glyphs;

    enum GlyphFlags : uint8_t {
        OnCurve = 0x01u,
        XShort = 0x02u,
        YShort = 0x04u,
        Repeat = 0x08u,
        XIsSameOrPositiveXShort = 0x10u,
        YIsSameOrPositiveYShort = 0x20u,
        OverlapSimple = 0x40u,
    };

    void Load(Stream& s) {
        s.SetIsLittleEndian(false);
        uint32_t sfntType = 0u;
        s.Serialize(sfntType);
        assert(sfntType == 0x00010000u || sfntType == 'OTTO'); // 0x00010000 = TrueType, 0x4F54544F 'OTTO' = CFF (version 1 or 2)
        uint16_t numTables = 0u;
        s.Serialize(numTables);
        uint16_t searchRange = 0u;
        s.Serialize(searchRange);
        uint16_t entrySelector = 0u;
        s.Serialize(entrySelector);
        uint16_t rangeShift = 0u;
        s.Serialize(rangeShift);
        Log::info("Loading .ttf numTables={}", numTables);

        struct Table {
            uint32_t tag;
            uint32_t checksum;
            uint32_t offset;
            uint32_t length;
            void Serialize(Stream& s) {
                s.Serialize(tag);
                s.Serialize(checksum);
                s.Serialize(offset);
                s.Serialize(length);
            }
        };

        Table tbHEAD;
        Table tbMAXP;
        Table tbLOCA;
        Table tbGLYF;
        Table tbCMAP;
        Table tbHHEA;
        Table tbHMTX;
        for(int i = 0; i < numTables; i++) {
            Table t = {};
            t.Serialize(s);
            if(t.tag == 'head') {
                tbHEAD = t;
            } else if(t.tag == 'maxp') {
                tbMAXP = t;
            } else if(t.tag == 'loca') {
                tbLOCA = t;
            } else if(t.tag == 'glyf') {
                tbGLYF = t;
            } else if(t.tag == 'cmap') {
                tbCMAP = t;
            } else if(t.tag == 'hhea') {
                tbHHEA = t;
            } else if(t.tag == 'hmtx') {
                tbHMTX = t;
            }
        }

        { // head
            s.SetPointer(tbHEAD.offset);
            head.Serialize(s);
        }
        { // maxp
            s.SetPointer(tbMAXP.offset);
            maxp.Serialize(s);
        }
        { // loca
            s.SetPointer(tbLOCA.offset);
            if(head.indexToLocFormat == 0) {
                for(int i = 0; i < maxp.numGlyphs; i++) {
                    uint16_t offset;
                    s.Serialize(offset);
                    offsets.push_back(uint32_t(offset)*2u);
                }
            } else {
                for(int i = 0; i < maxp.numGlyphs; i++) {
                    uint32_t offset;
                    s.Serialize(offset);
                    offsets.push_back(offset);
                }
            }
        }
        { // glyf
            for(int i = 0; i < maxp.numGlyphs; i++) {
                glyphs.push_back({});
                uint32_t offset = offsets[i];
                s.SetPointer(tbGLYF.offset + offset);
                int16 numContours;
                s.Serialize(numContours);
                int16 xMin;
                s.Serialize(xMin);
                int16 yMin;
                s.Serialize(yMin);
                int16 xMax;
                s.Serialize(xMax);
                int16 yMax;
                s.Serialize(yMax);
                std::vector<uint16_t> glyphEndsAt;
                for(int i = 0; i < numContours; i++) {
                    uint16_t end;
                    s.Serialize(end);
                    glyphEndsAt.push_back(end);
                }
                uint16_t instructionLength;
                s.Serialize(instructionLength);
                s.Skip(instructionLength);
                
                // empty character
                if(i+1 < maxp.numGlyphs && offsets[i+1] == offset) {
                    continue;
                }
                if(numContours < 0) {
                    continue;
                }

                uint32_t numPoints = glyphEndsAt.back() + 1u;
                std::vector<GlyphFlags> flags;
                for(uint32_t i = 0; i < numPoints; i++) {
                    uint8_t flag;
                    s.Serialize(flag);
                    flags.push_back(GlyphFlags(flag));
                    if(flag & GlyphFlags::Repeat) {
                        uint8_t repeatCount;
                        s.Serialize(repeatCount);
                        i+= repeatCount;
                        for(int i = 0; i < repeatCount; i++) {
                            flags.push_back(GlyphFlags(flag));
                        }
                    }
                }
                std::vector<int32_t> coordsX;
                SerializeCoords(s, flags, coordsX, GlyphFlags::XShort, GlyphFlags::XIsSameOrPositiveXShort);
                std::vector<int32_t> coordsY;
                SerializeCoords(s, flags, coordsY, GlyphFlags::YShort, GlyphFlags::YIsSameOrPositiveYShort);

                {
                    Glyph& glyph = glyphs.back();
                    glyph.min = glm::vec2(xMin, yMin) / float(head.unitsPerEm);
                    glyph.max = glm::vec2(xMax, yMax) / float(head.unitsPerEm);
                    std::vector<glm::vec2>& coords = glyph.coords;
                    uint32_t i = 0;
                    uint32_t c = 0;
                    while(i < numPoints) {
                        uint32_t ii = i + 1;
                        if(i == glyphEndsAt[c]) {
                            ii = (c == 0) ? 0 : (glyphEndsAt[c-1]+1);
                            c++;
                        }
                        bool onCurve1 = flags[i] & GlyphFlags::OnCurve;
                        bool onCurve2 = flags[ii] & GlyphFlags::OnCurve;
                        
                        coords.push_back(glm::vec2(coordsX[i], coordsY[i]));
                        if(onCurve1 && onCurve2) {
                            glm::vec2 c1 = glm::vec2(coordsX[i], coordsY[i]);
                            glm::vec2 c2 = glm::vec2(coordsX[ii], coordsY[ii]);
                            coords.push_back((c1+c2)*0.5f);
                            coords.push_back(glm::vec2(coordsX[ii], coordsY[ii]));
                        } else if(!onCurve1 && !onCurve2) {
                            glm::vec2 c1 = glm::vec2(coordsX[i], coordsY[i]);
                            glm::vec2 c2 = glm::vec2(coordsX[ii], coordsY[ii]);
                            coords.push_back((c1+c2)*0.5f);
                            coords.push_back((c1+c2)*0.5f);
                        } else if(!onCurve1 && onCurve2){
                            coords.push_back(glm::vec2(coordsX[ii], coordsY[ii]));
                        }

                        i++;
                    }
                    for(auto& c : coords) {
                        c /= float(head.unitsPerEm);
                    }
                }
                
            }
        }
        { // cmap
            s.SetPointer(tbCMAP.offset);
            uint16_t version;
            s.Serialize(version);
            uint16_t numTables;
            s.Serialize(numTables);
            struct Encoding {
                uint16_t platformID;
                uint16_t encodingID;
                uint32_t offset;
            };
            std::vector<Encoding> encodings = {};
            for(int i = 0; i < numTables; i++) {
                Encoding e;
                s.Serialize(e.platformID);
                s.Serialize(e.encodingID);
                s.Serialize(e.offset);
                encodings.push_back(e);
            }
            
            bool foundFormat12 = false;
            for(const auto& e : encodings) {
                s.SetPointer(tbCMAP.offset + e.offset);
                uint16_t format;
                s.Serialize(format);
                if(format == 12) {
                    foundFormat12 = true;
                    break;
                }
            }
            assert(foundFormat12);

            s.Skip(sizeof(uint16_t)); // reserved
            s.Skip(sizeof(uint32_t)); // length
            s.Skip(sizeof(uint32_t)); // language
            uint32_t numGroups;
            s.Serialize(numGroups);
            
            for(uint32_t i = 0; i < numGroups; i++) {
                uint32_t startCharCode;
                uint32_t endCharCode;
                uint32_t startGlyphId;
                s.Serialize(startCharCode);
                s.Serialize(endCharCode);
                s.Serialize(startGlyphId);
                 for(uint32_t j = 0; j <= endCharCode-startCharCode; j++) {
                    remap.emplace(startCharCode+j, startGlyphId+j);
                }
            }

        }
        { // hhea
            s.SetPointer(tbHHEA.offset);
            s.Skip(34);
            s.Serialize(numberOfHMetrics);
        }
        { // hmtx
            s.SetPointer(tbHMTX.offset);
            for(int i = 0; i < numberOfHMetrics; i++) {
                uint16_t advanceWidth;
                s.Serialize(advanceWidth);
                int16_t leftSideBearing;
                s.Serialize(leftSideBearing);
                auto& g = glyphs[i];
                g.leftSideBearing = leftSideBearing / float(head.unitsPerEm);
                g.advanceWidth = advanceWidth / float(head.unitsPerEm);
            }
        }

    }

    void SerializeCoords(Stream& s, const std::vector<GlyphFlags>& flags, std::vector<int32_t>& coords, GlyphFlags isShort, GlyphFlags isSameOrPositiveShort) {
        int32_t x = 0;
        for(uint32_t i = 0; i < flags.size(); i++) {
            GlyphFlags f = flags[i];
            if(f & isShort) {
                uint8_t v;
                s.Serialize(v);
                if(f & isSameOrPositiveShort) {
                    x += int32_t(v);
                } else {
                    x -= int32_t(v);
                }
            } else if(~f & isSameOrPositiveShort) {
                int16_t v;
                s.Serialize(v);
                x += int32_t(v);
            }
            coords.push_back(x);
        }
    }
};