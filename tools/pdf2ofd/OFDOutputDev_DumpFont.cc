#include <string>
#include <string.h>
#include <fstream>
#include <unordered_set>
#include <Object.h>
#include <CharTypes.h>
#include <CharCodeToUnicode.h>
#include <fofi/FoFiTrueType.h>
#include "OFDOutputDev.h"
#include "utils/logger.h"

using namespace std;

//using std::cerr;
//using std::endl;
//using std::ostream;

long long hash_ref(const Ref * id);

Unicode check_unicode(Unicode * u, int len, CharCode code, GfxFont * font);
Unicode unicode_from_font (CharCode code, GfxFont * font);

/**
 * Check whether a unicode character is illegal for the output HTML.
 * Unlike PDF readers, browsers has special treatments for such characters (normally treated as
 * zero-width space), regardless of metrics and glyphs provided by fonts. So these characters
 * should be mapped to unicode private area to "cheat" browsers, at the cost of losing actual
 * unicode values in the HTML.
 *
 * The following chart shows illegal characters  in HTML by webkit, mozilla, and pdf2htmlEX (p2h).
 * pdf2htmlEX's illegal character set is the union of webkit's and mozilla's, plus illegal unicode
 * characters. "[" and ")" surrounding ranges denote "inclusive" and "exclusive", respectively.
 *
 *         00(NUL)--09(\t)--0A(\n)--0D(\r)--20(SP)--7F(DEL)--9F(APC)--A0(NBSP)--AD(SHY)--061C(ALM)--1361(Ethiopic word space)
 * webkit:   [--------------------------------)        [------------------)       [-]
 * moz:      [--------------------------------)        [---------]                          [-]
 * p2h:      [--------------------------------)        [------------------]       [-]       [-]         [-]
 *
 *         200B(ZWSP)--200C(ZWNJ)--200D(ZWJ)--200E(LRM)--200F(RLM)--2028(LSEP)--2029(PSEP)--202A(LRE)--202E(RL0)--2066(LRI)--2069(PDI)
 * webkit:   [-----------------------------------------------]                                 [----------]
 * moz:      [-]                                  [----------]         [-]         [-]         [----------]         [------------]
 * p2h:      [-----------------------------------------------]         [-]         [-]         [----------]         [------------]
 *
 *         D800(surrogate)--DFFF(surrogate)--FEFF(ZWNBSP)--FFFC(ORC)--FFFE(non-char)--FFFF(non-char)
 * webkit:                                      [-]           [-]
 * moz:
 * p2h:         [------------------]            [-]           [-]          [-----------------]
 *
 * Note: 0xA0 (no-break space) affects word-spacing; and if "white-space:pre" is specified,
 * \n and \r can break line, \t can shift text, so they are considered illegal.
 *
 * Resources (retrieved at 2015-03-16)
 * * webkit
 *   * Avoid querying the font cache for the zero-width space glyph ( https://bugs.webkit.org/show_bug.cgi?id=90673 )
 *   * treatAsZeroWidthSpace( https://github.com/WebKit/webkit/blob/17bbff7400393e9389b40cc84ce005f7cc954680/Source/WebCore/platform/graphics/FontCascade.h#L272 )
 * * mozilla
 *   * IsInvalidChar( http://mxr.mozilla.org/mozilla-central/source/gfx/thebes/gfxTextRun.cpp#1973 )
 *   * IsBidiControl( http://mxr.mozilla.org/mozilla-central/source/intl/unicharutil/util/nsBidiUtils.h#114 )
 * * Character encodings in HTML ( http://en.wikipedia.org/wiki/Character_encodings_in_HTML#HTML_character_references )
 * * CSS Text Spec ( http://dev.w3.org/csswg/css-text/ )
 * * unicode table ( http://unicode-table.com )
 *
 * TODO Web specs? IE?
 *
 */
bool is_illegal_unicode(Unicode c) {
    return (c < 0x20) || (c >= 0x7F && c <= 0xA0) || (c == 0xAD)
            || (c == 0x061C) || (c == 0x1361)
            || (c >= 0x200B && c <= 0x200F) || (c == 0x2028) || (c == 0x2029)
            || (c >= 0x202A && c <= 0x202E) || (c >= 0x2066 && c <= 0x2069)
            || (c >= 0xD800 && c <= 0xDFFF) || (c == 0xFEFF) || (c == 0xFFFC)
            || (c == 0xFFFE) || (c == 0xFFFF);
}

Unicode map_to_private(CharCode code)
{
    Unicode private_mapping = (Unicode)(code + 0xE000);
    if(private_mapping > 0xF8FF)
    {
        private_mapping = (Unicode)((private_mapping - 0xF8FF) + 0xF0000);
        if(private_mapping > 0xFFFFD)
        {
            private_mapping = (Unicode)((private_mapping - 0xFFFFD) + 0x100000);
            if(private_mapping > 0x10FFFD)
            {
                cerr << "Warning: all private use unicode are used" << endl;
            }
        }
    }
    return private_mapping;
}

Unicode unicode_from_font (CharCode code, GfxFont * font)
{
    if(!font->isCIDFont())
    {
        char * cname = dynamic_cast<Gfx8BitFont*>(font)->getCharName(code);
        // may be untranslated ligature
        if(cname) {
            // FIXME
            Unicode ou = 0;
            //ou = globalParams->mapNameToUnicodeText(cname);
            if(!is_illegal_unicode(ou))
                return ou;
        }
    }

    return map_to_private(code);
}

Unicode check_unicode(Unicode * u, int len, CharCode code, GfxFont * font)
{
    if(len == 0)
        return map_to_private(code);

    if(len == 1)
    {
        if(!is_illegal_unicode(*u))
            return *u;
    }

    return unicode_from_font(code, font);
}
std::string OFDOutputDev::dump_embedded_font(GfxFont * font, XRef * xref) {
    //if(info.is_type3){
        //LOG(INFO) << "dump typ3 font.";
        //return "";
    //}

    Object obj, obj1, obj2;
    Object font_obj, font_obj2, fontdesc_obj;
    string suffix;
    string filepath;

    Ref *ref = font->getID();
    long long fn_id = ref->num;
    __attribute__((unused)) long long id1 = hash_ref(ref);


    try
    {
        // inspired by mupdf 
        string subtype;

        auto * id = font->getID();

        Object ref_obj;
        ref_obj.initRef(id->num, id->gen);
        ref_obj.fetch(xref, &font_obj);
        ref_obj.free();

        if(!font_obj.isDict())
        {
            cerr << "Font object is not a dictionary" << endl;
            throw 0;
        }

        Dict * dict = font_obj.getDict();
        if(dict->lookup("DescendantFonts", &font_obj2)->isArray())
        {
            if(font_obj2.arrayGetLength() == 0)
            {
                cerr << "Warning: empty DescendantFonts array" << endl;
            }
            else
            {
                if(font_obj2.arrayGetLength() > 1)
                    cerr << "TODO: multiple entries in DescendantFonts array" << endl;

                if(font_obj2.arrayGet(0, &obj2)->isDict())
                {
                    dict = obj2.getDict();
                }
            }
        }

        if(!dict->lookup("FontDescriptor", &fontdesc_obj)->isDict())
        {
            cerr << "Cannot find FontDescriptor " << endl;
            throw 0;
        }

        dict = fontdesc_obj.getDict();

        if(dict->lookup("FontFile3", &obj)->isStream())
        {
            if(obj.streamGetDict()->lookup("Subtype", &obj1)->isName())
            {
                subtype = obj1.getName();
                if(subtype == "Type1C")
                {
                    suffix = ".cff";
                }
                else if (subtype == "CIDFontType0C")
                {
                    suffix = ".cid";
                }
                else if (subtype == "OpenType")
                {
                    suffix = ".otf";
                }
                else
                {
                    cerr << "Unknown subtype: " << subtype << endl;
                    throw 0;
                }
            }
            else
            {
                cerr << "Invalid subtype in font descriptor" << endl;
                throw 0;
            }
        }
        else if (dict->lookup("FontFile2", &obj)->isStream())
        { 
            suffix = ".ttf";
        }
        else if (dict->lookup("FontFile", &obj)->isStream())
        {
            suffix = ".pfa";
        }
        else
        {
            cerr << "Cannot find FontFile for dump" << endl;
            throw 0;
        }

        if(suffix == "")
        {
            cerr << "Font type unrecognized" << endl;
            throw 0;
        }

        obj.streamReset();

        std::string tmpDir = "/tmp";
        filepath = tmpDir + "/f" + std::to_string(fn_id) + suffix;
        LOG(DEBUG) << "...... tmp font name: " << filepath << " fontid: " << fn_id;
        //tmp_files.add(filepath);

        ofstream outf(filepath.c_str(), ofstream::binary);
        if(!outf)
            throw string("Cannot open file ") + filepath + " for writing";

        char buf[1024];
        int len;
        while((len = obj.streamGetChars(1024, (Guchar*)buf)) > 0)
        {
            outf.write(buf, len);
        }
        obj.streamClose();
    }
    catch(int) 
    {
        cerr << "Something wrong when trying to dump font " << hex << fn_id << dec << endl;
    }

    obj2.free();
    obj1.free();
    obj.free();

    fontdesc_obj.free();
    font_obj2.free();
    font_obj.free();

    return filepath;
}


extern "C"{
#include "utils/ffw.h"
}

bool is_truetype_suffix(const std::string & suffix) {
    return (suffix == ".ttf") || (suffix == ".ttc") || (suffix == ".otf");
}

std::string get_filename (const std::string & path) {
    size_t idx = path.rfind('/');
    if(idx == std::string::npos)
        return path;
    else if (idx == path.size() - 1)
        return "";
    return path.substr(idx + 1);
}

std::string get_suffix(const std::string & path) {
    std::string fn = get_filename(path);
    size_t idx = fn.rfind('.');
    if(idx == string::npos)
        return "";
    else
    {
        std::string s = fn.substr(idx);
        for(auto & c : s)
            c = tolower(c);
        return s;
    }
}

#include <math.h>
bool equal(double x, double y) { return fabs(x-y) <= 0.000001; };

void OFDOutputDev::embed_font(const string & filepath, GfxFont * font, FontInfo & info, bool get_metric_only) {

    LOG(DEBUG) << "embed_font() filepath: " << filepath;
    assert(!filepath.empty());

    // Params
    int tounicode            = 1;      // how to handle ToUnicode CMaps. 
                                       // (0=auto, 1=force, -1=ignore)
    int stretch_narrow_glyph = 0;      // stretch narrow glyphs instead of padding them.
    int squeeze_wide_glyph   = 1;      // shrink wide glyphs instaed of truncating them.
    std::string tmp_dir      = "/tmp";
    std::string dest_dir     = ".";
    std::string external_hint_tool = ""; // external tool for hinting fonts (overrides --auto-hint)
    bool auto_hint = true;            // use fontforge autohint on fonts without hints
    bool override_fstype = false;     // clear the fstype bits in TTF/OTF fonts.
    bool embedFont = true;            // embed font files into output.
    std::string font_format = "woff"; // suffix for embedded font files (ttf, otf, woff, svg)

    //if(param.debug)
    //{
        //cerr << "Embed font: " << filepath << " " << info.id << endl;
    //}

    ffw_load_font(filepath.c_str());
    ffw_prepare_font();

    //if(param.debug)
    //{
        //auto fn = str_fmt("%s/__raw_font_%llx%s", param.tmp_dir.c_str(), info.id, get_suffix(filepath).c_str());
        //tmp_files.add((char*)fn);
        //ofstream((char*)fn, ofstream::binary) << ifstream(filepath).rdbuf();
    //}

    int * code2GID = nullptr;
    int code2GID_len = 0;
    int maxcode = 0;

    Gfx8BitFont * font_8bit = nullptr;
    GfxCIDFont * font_cid = nullptr;

    string suffix = get_suffix(filepath);
    for(auto & c : suffix)
        c = tolower(c);

    /*
     * if parm->tounicode is 0, try the provided tounicode map first
     */
    // TODO
    info.use_tounicode = tounicode >= 0;
    bool has_space = false;

    const char * used_map = nullptr;

    info.em_size = ffw_get_em_size();

    //if(param.debug)
    //{
        //cerr << "em size: " << info.em_size << endl;
    //}

    info.space_width = 0;

    if(!font->isCIDFont()) {
        font_8bit = dynamic_cast<Gfx8BitFont*>(font);
    } else {
        font_cid = dynamic_cast<GfxCIDFont*>(font);
    }

    if(get_metric_only) {
        ffw_fix_metric();
        ffw_get_metric(&info.ascent, &info.descent);
        ffw_close();
        return;
    }

    // FIXME
    //used_map = m_preprocessor.get_code_map(hash_ref(font->getID()));

    /*
     * Step 1
     * dump the font file directly from the font descriptor and put the glyphs into the correct slots *
     *
     * for 8bit + nonTrueType
     * re-encoding the font by glyph names
     *
     * for 8bit + TrueType
     * sort the glyphs as the original order, and load the code2GID table
     * later we will map GID (instead of char code) to Unicode
     *
     * for CID + nonTrueType
     * Flatten the font 
     *
     * for CID Truetype
     * same as 8bitTrueType, except for that we have to check 65536 charcodes
     * use the embedded code2GID table if there is, otherwise use the one in the font
     */
    if(font_8bit){
        maxcode = 0xff;
        if(is_truetype_suffix(suffix)) {
            //if(info.is_type3) {
                 //Type 3 fonts are saved and converted into ttf fonts
                 //encoded based on code points instead of GID
                
                 //I thought code2GID would work but it never works, and I don't know why
                 //Anyway we can disable code2GID such that the following procedure will be working based on code points instead of GID
            //}
            //else
            {
                ffw_reencode_glyph_order();
                if(FoFiTrueType * fftt = FoFiTrueType::load((char*)filepath.c_str())) {
                    code2GID = font_8bit->getCodeToGIDMap(fftt);
                    code2GID_len = 256;
                    delete fftt;
                }
            }
        } else {
            // move the slot such that it's consistent with the encoding seen in PDF
            unordered_set<string> nameset;
            bool name_conflict_warned = false;

            std::fill(cur_mapping2.begin(), cur_mapping2.end(), (char*)nullptr);

            for(int i = 0; i < 256; ++i) {
                if(!used_map[i]) continue;

                auto cn = font_8bit->getCharName(i);
                if(cn == nullptr) {
                    continue;
                } else {
                    if(nameset.insert(string(cn)).second) {
                        cur_mapping2[i] = cn;    
                    } else {
                        if(!name_conflict_warned) {
                            name_conflict_warned = true;
                            //TODO: may be resolved using advanced font properties?
                            cerr << "Warning: encoding conflict detected in font: " << hex << info.id << dec << endl;
                        }
                    }
                }
            }

            ffw_reencode_raw2(cur_mapping2.data(), 256, 0);
        }
    } else {
        maxcode = 0xffff;

        if(is_truetype_suffix(suffix)) {
            ffw_reencode_glyph_order();

            GfxCIDFont * _font = dynamic_cast<GfxCIDFont*>(font);

            // To locate CID2GID for the font
            // as in CairoFontEngine.cc
            if((code2GID = _font->getCIDToGID())) {
                // use the mapping stored in _font
                code2GID_len = _font->getCIDToGIDLen();
            } else {
                // use the mapping stored in the file
                if(FoFiTrueType * fftt = FoFiTrueType::load((char*)filepath.c_str())) {
                    code2GID = _font->getCodeToGIDMap(fftt, &code2GID_len);
                    delete fftt;
                }
            }
        } else {
            // TODO: add an option to load the table?
            ffw_cidflatten();
        }
    }

    /*
     * Step 2
     * - map charcode (or GID for CID truetype)
     *
     * -> Always map to Unicode for 8bit TrueType fonts and CID fonts
     *
     * -> For 8bit nonTruetype fonts:
     *   Try to calculate the correct Unicode value from the glyph names, when collision is detected in ToUnicode Map
     * 
     * - Fill in the width_list, and set widths accordingly
     */


    {
        //string map_filename;
        //ofstream map_outf;
        //if(param.debug)
        //{
            //map_filename = (char*)str_fmt("%s/f%llx.map", param.tmp_dir.c_str(), info.id);
            //tmp_files.add(map_filename);
            //map_outf.open(map_filename);
        //}

        unordered_set<int> codeset;
        bool name_conflict_warned = false;

        auto ctu = font->getToUnicode();
        std::fill(cur_mapping.begin(), cur_mapping.end(), -1);
        std::fill(width_list.begin(), width_list.end(), -1);

        if(code2GID){
            maxcode = min<int>(maxcode, code2GID_len - 1);
        }

        bool is_truetype = is_truetype_suffix(suffix);
        int max_key = maxcode;
        /*
         * Traverse all possible codes
         */
        bool retried = false; // avoid infinite loop
        for(int cur_code = 0; cur_code <= maxcode; ++cur_code) {
            if(!used_map[cur_code]) continue;

            /*
             * Skip glyphs without names (only for non-ttf fonts)
             */
            if(!is_truetype && (font_8bit != nullptr) 
                    && (font_8bit->getCharName(cur_code) == nullptr)) {
                continue;
            }

            int mapped_code = cur_code;
            if(code2GID) {
                // for fonts with GID (e.g. TTF) we need to map GIDs instead of codes
                if((mapped_code = code2GID[cur_code]) == 0) continue;
            }

            if(mapped_code > max_key){
                max_key = mapped_code;
            }

            Unicode u, *pu=&u;
            if(info.use_tounicode) {
                int n = ctu ? (ctu->mapToUnicode(cur_code, &pu)) : 0;
                u = check_unicode(pu, n, cur_code, font);
            } else {
                u = unicode_from_font(cur_code, font);
            }

            if(codeset.insert(u).second) {
                cur_mapping[mapped_code] = u;
            } else {
                // collision detected
                if( tounicode == 0 ) {
                    // in auto mode, just drop the tounicode map
                    if(!retried) {
                        LOG(ERROR) << "ToUnicode CMap is not valid and got dropped for font: " << hex << info.id << dec << endl;
                        retried = true;
                        codeset.clear();
                        info.use_tounicode = false;
                        std::fill(cur_mapping.begin(), cur_mapping.end(), -1);
                        std::fill(width_list.begin(), width_list.end(), -1);
                        cur_code = -1;
                        //if(param.debug)
                        //{
                            //map_outf.close();
                            //map_outf.open(map_filename);
                        //}
                        continue;
                    }
                }
                if(!name_conflict_warned) {
                    name_conflict_warned = true;
                    //TODO: may be resolved using advanced font properties?
                    LOG(ERROR) << "Warning: encoding confliction detected in font: " << hex << info.id << dec << endl;
                }
            }

            {
                double cur_width = 0;
                if(font_8bit) {
                    cur_width = font_8bit->getWidth(cur_code);
                } else {
                    char buf[2];  
                    buf[0] = (cur_code >> 8) & 0xff;
                    buf[1] = (cur_code & 0xff);
                    cur_width = font_cid->getWidth(buf, 2) ;
                }

                cur_width /= info.font_size_scale;

                if(u == ' ') {
                    /*
                     * Internet Explorer will ignore `word-spacing` if
                     * the width of the 'space' glyph is 0
                     *
                     * space_width==0 often means no spaces are used in the PDF
                     * so setting it to be 0.001 should be safe
                     */
                    if(equal(cur_width, 0))
                        Unicode unicode_from_font (CharCode code, GfxFont * font);
                    cur_width = 0.001;

                    info.space_width = cur_width;
                    has_space = true;
                }
                
                width_list[mapped_code] = (int)floor(cur_width * info.em_size + 0.5);
            }

            //if(param.debug)
            //{
                //map_outf << hex << cur_code << ' ' << mapped_code << ' ' << u << dec << endl;
            //}
        }

        // FIXME
        ffw_set_widths(width_list.data(), max_key + 1, stretch_narrow_glyph, squeeze_wide_glyph);
        
        ffw_reencode_raw(cur_mapping.data(), max_key + 1, 1);

        // In some space offsets in HTML, we insert a ' ' there in order to improve text copy&paste
        // We need to make sure that ' ' is in the font, otherwise it would be very ugly if you select the text
        // Might be a problem if ' ' is in the font, but not empty
        if(!has_space) {
            if(font_8bit) {
                info.space_width = font_8bit->getWidth(' ');
            } else {
                char buf[2] = {0, ' '};
                info.space_width = font_cid->getWidth(buf, 2);
            }
            info.space_width /= info.font_size_scale;

            /* See comments above */
            if(equal(info.space_width,0)){
                info.space_width = 0.001;
            }

            ffw_add_empty_char((int32_t)' ', (int)floor(info.space_width * info.em_size + 0.5));
            //if(param.debug)
            //{
                //cerr << "Missing space width in font " << hex << info.id << ": set to " << dec << info.space_width << endl;
            //}
        }

        //if(param.debug)
        //{
            //cerr << "space width: " << info.space_width << endl;
        //}

        if(ctu){
            ctu->decRefCnt();
        }
    }

    /*
     * Step 3
     * Generate the font as desired
     */

    // Reencode to Unicode Full such that FontForge won't ditch unicode values larger than 0xFFFF
    ffw_reencode_unicode_full();

    // Due to a bug of Fontforge about pfa -> woff conversion
    // we always generate TTF first, instead of the format specified by user

    // FIXME
    std::string cur_tmp_fn = (char*)str_fmt("%s/__tmp_font1.%s", tmp_dir.c_str(), "ttf");
    //tmp_files.add(cur_tmp_fn);
    std::string other_tmp_fn = (char*)str_fmt("%s/__tmp_font2.%s", tmp_dir.c_str(), "ttf");
    //tmp_files.add(other_tmp_fn);

    ffw_save(cur_tmp_fn.c_str());

    ffw_close();

    /*
     * Step 4
     * Font Hinting
     */
    bool hinted = false;

    // Call external hinting program if specified 
    // FIXME
    if(external_hint_tool != ""){
        hinted = (system((char*)str_fmt("%s \"%s\" \"%s\"", external_hint_tool.c_str(), cur_tmp_fn.c_str(), other_tmp_fn.c_str())) == 0);
    }

    // Call internal hinting procedure if specified 
    if((!hinted) && (auto_hint))
    {
        ffw_load_font(cur_tmp_fn.c_str());
        ffw_auto_hint();
        ffw_save(other_tmp_fn.c_str());
        ffw_close();
        hinted = true;
    }

    if(hinted) {
        swap(cur_tmp_fn, other_tmp_fn);
    }

    /* 
     * Step 5 
     * Generate the font, load the metrics and set the embedding bits (fstype)
     *
     * Ascent/Descent are not used in PDF, and the values in PDF may be wrong or inconsistent (there are 3 sets of them)
     * We need to reload in order to retrieve/fix accurate ascent/descent, some info won't be written to the font by fontforge until saved.
     */
    // FIXME
    std::string fn = (char*)str_fmt("%s/f%llx.%s", 
        (embedFont ? tmp_dir : dest_dir).c_str(),
        info.id, font_format.c_str());

    // FIXME
    //if( embedFont ){
        //tmp_files.add(fn);
    //}

    ffw_load_font(cur_tmp_fn.c_str());
    ffw_fix_metric();
    ffw_get_metric(&info.ascent, &info.descent);

    // FIXME
    if ( override_fstype ){
        ffw_override_fstype();
    }

    ffw_save(fn.c_str());

    ffw_close();
}

void OFDOutputDev::install_embedded_font(GfxFont * font, FontInfo & info) {
    // FIXME
    //auto path = dump_embedded_font(font, info);
    auto path = dump_embedded_font(font, m_xref);

    if(path != "") {
        embed_font(path, font, info);
        //export_remote_font(info, param.font_format, font);
    } else {
        //export_remote_default_font(info.id);
    }
}

const map<string, string> GB_ENCODED_FONT_NAME_MAP({
    {"\xCB\xCE\xCC\xE5", "SimSun"},
    {"\xBA\xDA\xCC\xE5", "SimHei"},
    {"\xBF\xAC\xCC\xE5_GB2312", "SimKai"},
    {"\xB7\xC2\xCB\xCE_GB2312", "SimFang"},
    {"\xC1\xA5\xCA\xE9", "SimLi"},
});

void OFDOutputDev::install_external_font(GfxFont *font, FontInfo & info) {
    bool embed_external_font = true;

    string fontname(font->getName()->getCString());

    // resolve bad encodings in GB
    auto iter = GB_ENCODED_FONT_NAME_MAP.find(fontname); 
    if(iter != GB_ENCODED_FONT_NAME_MAP.end()) {
        fontname = iter->second;
        LOG(ERROR) << "Warning: workaround for font names in bad encodings.";
    }

    GfxFontLoc * localfontloc = font->locateFont(m_xref, nullptr);

    if(embed_external_font) {
        if(localfontloc != nullptr) {
            embed_font(string(localfontloc->path->getCString()), font, info);
            delete localfontloc;
            return;
        } else {
            LOG(ERROR) << "Cannot embed external font: f" << hex << info.id << dec << ' ' << fontname;
            // fallback to exporting by name
        }
    }

    // still try to get an idea of read ascent/descent
    if(localfontloc != nullptr) {
        // fill in ascent/descent only, do not embed
        embed_font(string(localfontloc->path->getCString()), font, info, true);
        delete localfontloc;
    } else {
        info.ascent = font->getAscent();
        info.descent = font->getDescent();
    }

    //export_local_font(info, font, fontname, "");
}


const FontInfo * OFDOutputDev::install_font(GfxFont * font) {
    assert(sizeof(long long) == 2*sizeof(int));
                
    long long fn_id = (font == nullptr) ? 0 : hash_ref(font->getID());

    // FIXME
    std::unordered_map<long long, FontInfo> font_info_map;

    auto iter = font_info_map.find(fn_id);
    if(iter != font_info_map.end())
        return &(iter->second);

    long long new_fn_id = font_info_map.size(); 

    auto cur_info_iter = font_info_map.insert(make_pair(fn_id, FontInfo())).first;

    FontInfo & new_font_info = cur_info_iter->second;
    new_font_info.id = new_fn_id;
    new_font_info.use_tounicode = true;
    new_font_info.font_size_scale = 1.0;

    if(font == nullptr)
    {
        new_font_info.em_size = 0;
        new_font_info.space_width = 0;
        new_font_info.ascent = 0;
        new_font_info.descent = 0;
        new_font_info.is_type3 = false;

        //export_remote_default_font(new_fn_id);

        return &(new_font_info);
    }

    new_font_info.ascent = font->getAscent();
    new_font_info.descent = font->getDescent();
    new_font_info.is_type3 = (font->getType() == fontType3);

    //if(param.debug)
    //{
        //cerr << "Install font " << hex << new_fn_id << dec
            //<< ": (" << (font->getID()->num) << ' ' << (font->getID()->gen) << ") " 
            //<< (font->getName() ? font->getName()->getCString() : "")
            //<< endl;
    //}

    if(new_font_info.is_type3)
    {
#if ENABLE_SVG
        //if(param.process_type3)
        if ( true)
        {
            install_embedded_font(font, new_font_info);
        }
        else
        {
            //export_remote_default_font(new_fn_id);
        }
#else
        cerr << "Type 3 fonts are unsupported and will be rendered as Image" << endl;
        //export_remote_default_font(new_fn_id);
#endif
        return &new_font_info;
    }
    if(font->getWMode()) {
        cerr << "Writing mode is unsupported and will be rendered as Image" << endl;
        //export_remote_default_font(new_fn_id);
        return &new_font_info;
    }

    /*
     * The 2nd parameter of locateFont should be true only for PS
     * which does not make much sense in our case
     * If we specify gFalse here, font_loc->locType cannot be gfxFontLocResident
     */
    if(auto * font_loc = font->locateFont(m_xref, nullptr))
    {
        switch(font_loc -> locType)
        {
            case gfxFontLocEmbedded:
                install_embedded_font(font, new_font_info);
                break;
            case gfxFontLocResident:
                std::cerr << "Warning: Base 14 fonts should not be specially handled now. Please report a bug!" << std::endl;
                /* fall through */
            case gfxFontLocExternal:
                install_external_font(font, new_font_info);
                break;
            default:
                cerr << "TODO: other font loc" << endl;
                //export_remote_default_font(new_fn_id);
                break;
        }      
        delete font_loc;
    }
    else
    {
        //export_remote_default_font(new_fn_id);
    }
      
    return &new_font_info;
}
