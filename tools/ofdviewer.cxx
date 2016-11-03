#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cairo/cairo.h>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "logger.h"

bool fullScreen = false;


// -------- create_cairo_surface_from_sdl_surface() --------
cairo_surface_t *create_cairo_surface_from_sdl_surface(SDL_Surface *sdl_surface){
    cairo_surface_t *cairo_surface = nullptr;

    cairo_surface = cairo_image_surface_create_for_data(
            (unsigned char*)sdl_surface->pixels,
            CAIRO_FORMAT_ARGB32,
            sdl_surface->w,
            sdl_surface->h,
            sdl_surface->pitch
            );

    return cairo_surface;
}

// -------- loadTexture() --------
SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &image_file){
    SDL_Texture *texture = nullptr;

    SDL_Surface *loadedSurface = IMG_Load(image_file.c_str());
    if ( loadedSurface != nullptr ){
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if ( texture == nullptr ){
            LOG(WARNING) << "SDL_CreateTextureFromSurface() failed. " << SDL_GetError();
        }
        SDL_FreeSurface(loadedSurface);
    } else {
        LOG(WARNING) << "IMG_Load(\"" << image_file << "\") failed " << SDL_GetError();
    }

    return texture;
}


SDL_Surface *create_image_surface(int width, int height, int bpp){
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000; 
    gmask = 0x00ff0000; 
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000; 
    amask = 0xff000000; 
#endif

    SDL_Surface *imageSurface = SDL_CreateRGBSurface(0, width, height, bpp, 
            rmask, gmask, bmask, amask);
    if ( imageSurface == nullptr ){
        LOG(ERROR) << "SDL_CreateRGBSurface() failed. " << SDL_GetError();
    }

    return imageSurface;
}

using namespace ofd;
//int main(){
int main(int argc, char *argv[]){

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];


    OFDPackage package;
    OFDDocumentPtr document = nullptr;
    OFDPagePtr currentPage = nullptr;
    size_t totalPages = 0;
    size_t showPage = 0;
    if ( package.Open(argv[1]) ){
        document = OFDDocumentPtr(package.GetOFDDocument()); 
        //OFDDocumentPtr document = package.GetOFDDocument(); 

        LOG(DEBUG) << document->String();

        size_t n_pages = document->GetPagesCount();
        totalPages = n_pages;
        LOG(INFO) << "Loading " << n_pages << " pages.";

        //for ( size_t i = 0 ; i < n_pages ; i++ ){
            //TIMED_SCOPE(timerDrawPage, "Draw Page");

            //OFDPagePtr page = document->GetOFDPage(i);
            ////page = document->GetOFDPage(i);
            //page->Open();

            //VLOG(3) << page->String(); 
            //LOG(INFO) << page->GetText();

            ////std::stringstream ss;
            ////ss << "Page" << (i + 1) << ".png";
            ////std::string png_filename = ss.str();
            ////page->RenderToPNGFile(png_filename);

            ////page->Close();
        //}
        currentPage = document->GetOFDPage(0);
        currentPage->Open();

    } else {
        LOG(ERROR) << "package.Open() failed. " << argv[1];
        return -1;
    }


    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
        LOG(ERROR) << "Failed to initialize SDL. " << SDL_GetError();
        return -1;
    }

    if ( !SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") ){
        LOG(ERROR) << "SDL_SetHint() failed. " << SDL_GetError();
        return -1;
    }

    int screenWidth = 794;
    int screenHeight = 1122;
    int screenFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE; // SDL_WINDOW_BORDERLESS
    int screenBPP = 32;
    SDL_Window *mainWindow = SDL_CreateWindow("OFD Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, screenFlags);
    if ( mainWindow == nullptr ){
        LOG(ERROR) << "SDL_CreateWindow() failed. " << SDL_GetError();
        return -1;
    }

    //SDL_Surface *primarySurface = SDL_GetWindowSurface(mainWindow);
    //cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(primarySurface);
    //if ( cairoSurface == nullptr ){
        //LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
        //return -1;
    //}

    SDL_Renderer *screenRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
    if ( screenRenderer == nullptr ){
        LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
        return -1;
    }

    if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ){
        LOG(ERROR) << "IMG_Init() failed. " << SDL_GetError();
        return -1;
    }


    SDL_Surface *imageSurface = create_image_surface(screenWidth, screenHeight, screenBPP);

    bool done = false;
    while ( !done ){
        SDL_Event event;
        while ( SDL_PollEvent(&event) != 0 ){
            if ( event.type == SDL_QUIT ){
                done = true;
            }
            // OnEvent(event);
            switch (event.type) {
            //case SDL_TEXTINPUT:
                //break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    done = true;
                } else if (event.key.keysym.sym == SDLK_q){
                    //SDL_GetModState() & KMOD_CTRL
                    //inputText = SDL_GetClipboardText();
                    done = true;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    if ( showPage == 0 ){
                        showPage = totalPages - 1;
                    } else {
                        showPage--;
                    }
                    currentPage = document->GetOFDPage(showPage);
                    currentPage->Open();
                    break;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    if ( showPage < totalPages - 1 ){
                        showPage++;
                    } else {
                        showPage = 0;
                    }
                    currentPage = document->GetOFDPage(showPage);
                    currentPage->Open();
                    break;
                } else if (event.key.keysym.sym == SDLK_RETURN) {
                    if (fullScreen){
                        SDL_SetWindowFullscreen(mainWindow, SDL_FALSE);
                        fullScreen = false;
                    } else {
                        SDL_SetWindowFullscreen(mainWindow, SDL_TRUE);
                        fullScreen = true;
                    }
                }
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                }
                break;
            case SDL_WINDOWEVENT:
                switch ( event.window.event ){
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screenWidth = event.window.data1;
                    screenHeight = event.window.data2;

                    //SDL_DestroyRenderer(screenRenderer);
                    //screenRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
                    //if ( screenRenderer == nullptr ){
                        //LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
                        //return -1;
                    //}

                    SDL_FreeSurface(imageSurface);
                    imageSurface = create_image_surface(screenWidth, screenHeight, screenBPP);
                    SDL_RenderPresent(screenRenderer);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(screenRenderer);
                    break;

                case SDL_WINDOWEVENT_ENTER:
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    break;
                };
                break;
            };
        };

        // Loop()
        SDL_FillRect(imageSurface, NULL, 0xFFFFFF);


        cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(imageSurface);
        if ( cairoSurface == nullptr ){
            LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
            return -1;
        }

        if ( currentPage != nullptr ){
            currentPage->Render(cairoSurface);
        }

        // Render(imageSurface)
        //SDL_Texture *backgroundTexture = SDL_CreateTexture(screenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
        //if ( backgroundTexture == nullptr ){
            //LOG(ERROR) << "SDL_CreateTexture() failed. " << SDL_GetError();
            //return -1;
        //}
        //SDL_UpdateTexture(backgroundTexture, NULL, imageSurface->pixels, imageSurface->pitch);

        SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(screenRenderer, imageSurface);

        // Clear screen
        SDL_SetRenderDrawColor(screenRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(screenRenderer);
        // Render texture to screen
        SDL_Rect destRect{0, 0, screenWidth, screenHeight};
        SDL_RenderCopy(screenRenderer, backgroundTexture, NULL, &destRect);

        // Update screen
        SDL_RenderPresent(screenRenderer);

        SDL_DestroyTexture(backgroundTexture);

        SDL_Delay(1); // Breath
    };

    // Cleanup()
    SDL_FreeSurface(imageSurface);
    if ( screenRenderer != nullptr ){
        SDL_DestroyRenderer(screenRenderer);
        screenRenderer = nullptr;
    }

    if ( mainWindow != nullptr ){
        SDL_DestroyWindow(mainWindow);
        mainWindow = nullptr;
    }

    IMG_Quit();
    SDL_Quit();

    package.Close();

    LOG(INFO) << "Done.";

    return 0;
}

