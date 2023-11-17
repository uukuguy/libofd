
// http://mupdf.com/docs/browse/source/tools/pdfextract.c

//#include <mupdf/pdf.h>
#include "utils/logger.h"



void test_mupdf(int argc, char *argv[]){
    //pdf_document *doc = nullptr;

    //fz_context *ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    //if ( ctx == nullptr ) exit(-1);

    //doc = pdf_open_document(ctx, argv[1]);
    //if ( pdf_needs_password(ctx, doc) ){
    //}

    //size_t num_objects = pdf_count_objects(ctx, doc);
    //for ( auto n = 1 ; n < num_objects ; n++ ){

        //fz_try(ctx){
            //pdf_obj *obj = pdf_load_object(ctx, doc, n, 0);

            //pdf_obj *objType = pdf_dict_get(ctx, obj, PDF_NAME_Type);
            //bool isFontDesc = pdf_name_eq(ctx, objType, PDF_NAME_FontDescriptor);

            //pdf_obj *subType = pdf_dict_get(ctx, obj, PDF_NAME_Subtype);
            //bool isImage = pdf_name_eq(ctx, subType, PDF_NAME_Image);

            //if ( isImage ){
                //fz_image *image;
                //fz_pixmap *pix;
                //pdf_obj *ref;
                //char buf[32];

                //ref = pdf_new_indirect(ctx, doc, n, 0);
                //image = pdf_load_image(ctx, doc, ref);
                //pix = fz_get_pixmap_from_image(ctx, image, 0, 0);
                //fz_drop_image(ctx, image);

                //snprintf(buf, sizeof(buf), "img-%04d", n);
                ////fz_write_pixmap_as_png(ctx, pix, buf, dorgb);
                //fz_drop_pixmap(ctx, pix);
                //pdf_drop_obj(ctx, ref);
            //} else if ( isFontDesc ){

            //}
            //pdf_drop_obj(ctx, obj);
        //} fz_catch(ctx){
            //fz_warn(ctx, "ignoring object %d", n);
        //}

    //}

    //pdf_drop_document(ctx, doc);
    //fz_flush_warnings(ctx);
    //fz_drop_context(ctx);

}

