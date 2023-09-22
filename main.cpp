#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <gd.h>
#include <gdfonts.h>
#include "nlohmann/json.hpp"
#include <gdfontl.h>
#include <curl/curl.h>
#include <algorithm>
#include <filesystem>
#include <limits>
#include <iomanip>
#include <map>
#include <ft2build.h>
#include <filesystem>
#include FT_FREETYPE_H
#include FT_GLYPH_H

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

const string data_url = "https://data.opendatasoft.com/api/explore/v2.1/catalog/datasets/emissions-co2-territoriales-par-secteur-france@akajoule/records?limit=20";
const string json_filename = "CO2_data.json";

// Définir une structure de données pour stocker les paires (année, taux)
struct DataPoint
{
    string year;
    string subsector;
    double rate;
};

// Structure pour stocker les données par année et sous-secteur
struct YearData
{
    vector<string> subsectors;
    vector<double> rates;
};

// Fonction de comparaison pour trier les données par année
bool compareByYear(const DataPoint &a, const DataPoint &b)
{
    return a.year < b.year;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    ofstream *output_stream = static_cast<ofstream *>(userp);

    if (output_stream)
    {
        output_stream->write(static_cast<const char *>(contents), total_size);
    }

    return total_size;
}

void getJson()
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        ofstream json_file(json_filename, ofstream::binary);

        if (json_file.is_open())
        {
            curl_easy_setopt(curl, CURLOPT_URL, data_url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_file);

            CURLcode res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
                cerr << "Error: Failed to download file. " << curl_easy_strerror(res) << endl;
            }

            json_file.close();
        }
        else
        {
            cerr << "Error: Failed to open file for writing." << endl;
        }

        curl_easy_cleanup(curl);
    }
    else
    {
        cerr << "Error: Failed to initialize cURL." << endl;
    }
}

void useArialFont(gdImagePtr im)
{
    FT_Library library;
    FT_Face face;

    if (FT_Init_FreeType(&library) != 0)
    {
        cerr << "Error: Failed to initialize FreeType library." << endl;
        return;
    }

    // Chemin complet de la police Arial
    string fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"; // Changez le chemin vers la police Arial sur votre système

    if (FT_New_Face(library, fontPath.c_str(), 0, &face) != 0)
    {
        cerr << "Error: Failed to load the Arial font." << endl;
        FT_Done_FreeType(library);
        return;
    }

    gdFTUseFontConfig(1);

    // Définir la couleur du texte
    int textColor = gdImageColorAllocate(im, 0, 0, 0);

    // L'utilisation de la police Arial est correcte
    gdImageStringTTF(im, nullptr, textColor, fontPath.c_str(), 12.0, 0.0, 10, 10, "Arial Font Test");

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void histogramme_CO2(const string &filename, const string &title, const vector<string> &years, const vector<double> &rates)
{
    const int image_width = 1200; // Augmentez la largeur de l'image
    const int image_height = 800;
    int barWidth = 40;   // Augmentez la largeur de chaque barre
    int barSpacing = 60; // Augmentez l'espacement entre les barres
    int totalBarWidth = (barWidth + barSpacing) * years.size() - barSpacing;
    int startX = 100;
    int startY = image_height - 100;

    gdImagePtr im = gdImageCreateTrueColor(image_width, image_height);
    int white = gdImageColorAllocate(im, 255, 255, 255);
    int black = gdImageColorAllocate(im, 0, 0, 0);
    int textColor = gdImageColorAllocate(im, 0, 0, 0);

    gdImageFill(im, 0, 0, white);

    int titleX = (image_width - gdFontGetLarge()->w * title.length()) / 2;
    gdImageString(im, gdFontGetLarge(), titleX, 50, (unsigned char *)title.c_str(), textColor);
    gdImageStringUp(im, gdFontGetSmall(), 20, startY - 350, (unsigned char *)"CO2 rate (g/kWh)", textColor);

    gdImageLine(im, startX, startY, image_width - 100, startY, black);
    gdImageLine(im, startX, startY, startX, startY - 500, black);

    int numLabels = 5;
    int labelIncrement = 20;
    int startYValue = 0;

    for (int i = 0; i <= numLabels; i++)
    {
        int labelValue = startYValue + i * labelIncrement;
        int labelY = startY - static_cast<int>((labelValue - 0) / 100.0 * 500);

        stringstream labelStream;
        labelStream << labelValue << "%";

        gdImageString(im, gdFontGetSmall(), startX - 40, labelY, (unsigned char *)labelStream.str().c_str(), textColor);
    }

    useArialFont(im);

    int titleY1 = startY + 20;
    int titleY2 = titleY1 + 15;
    bool useTitleY1 = true;

    for (int i = 0; i < years.size(); i++)
    {
        double normalizedRate = (rates[i] <= 100.0) ? rates[i] : 100.0;
        int barHeight = static_cast<int>((normalizedRate / 100.0) * 500);
        int x1 = startX + (barWidth + barSpacing) * i;
        int y1 = startY - barHeight;
        int x2 = x1 + barWidth;
        int y2 = startY;

        int color = gdImageColorAllocate(im, 0, 0, 255);
        gdImageFilledRectangle(im, x1, y1, x2, y2, color);

        int titleX = x1 + barWidth / 2 - gdFontGetSmall()->w / 2;
        int titleY = useTitleY1 ? titleY1 : titleY2;
        useTitleY1 = !useTitleY1;

        gdImageString(im, gdFontGetSmall(), titleX, titleY, (unsigned char *)years[i].c_str(), textColor);
    }

    FILE *out = fopen(filename.c_str(), "wb");
    if (out != nullptr)
    {
        gdImagePng(im, out);
        fclose(out);
    }
    else
    {
        cerr << "Error: Failed to open output file for writing." << endl;
    }

    gdImageDestroy(im);
}

int main()
{
    if (!fs::is_directory("graphique"))
    {
        if (!fs::create_directory("graphique"))
        {
            cerr << "Error: Failed to create the 'graphique' directory." << endl;
            return 1; // Quittez le programme avec un code d'erreur
        }
    }

    getJson();
    cout << "Download complete " << json_filename << " has been retrieved." << endl;
    ifstream file(json_filename);
    json data;
    file >> data;

    vector<DataPoint> dataPoints;

    if (data.contains("results") && data["results"].is_array())
    {
        for (const auto &result : data["results"])
        {
            if (result.contains("valeur") && result["valeur"].is_number())
            {
                DataPoint dataPoint;
                dataPoint.year = result["annee"].get<string>();
                dataPoint.subsector = result["sous_secteur"].get<string>();
                dataPoint.rate = result["valeur"].get<double>();
                dataPoints.push_back(dataPoint);
            }
        }
    }

    // Calculez la valeur minimale (minCO2) et la valeur maximale (maxCO2) des émissions de CO2 à partir de vos données
    double minCO2 = std::numeric_limits<double>::max();
    double maxCO2 = std::numeric_limits<double>::min();

    for (const DataPoint &dataPoint : dataPoints)
    {
        if (dataPoint.rate < minCO2)
        {
            minCO2 = dataPoint.rate;
        }
        if (dataPoint.rate > maxCO2)
        {
            maxCO2 = dataPoint.rate;
        }
    }

    // Structure pour stocker les données par année et sous-secteur
    std::map<string, YearData> yearDataMap;

    // Remplacez la boucle de filtrage des données actuelle
    for (const DataPoint &dataPoint : dataPoints)
    {
        // Regroupez les données par année et sous-secteur
        yearDataMap[dataPoint.year].subsectors.push_back(dataPoint.subsector);
        yearDataMap[dataPoint.year].rates.push_back(dataPoint.rate);
    }

    // Générez un histogramme pour chaque année
    for (const auto &yearDataPair : yearDataMap)
    {
        const string &year = yearDataPair.first;
        const YearData &yearData = yearDataPair.second;

        // Spécifiez le chemin complet du fichier de sortie dans le dossier "graphique"
        string filename = "graphique/emissionCO2_" + year + ".png";

        // Générez le graphique pour cette année avec tous les sous-secteurs
        histogramme_CO2(filename, year, yearData.subsectors, yearData.rates);
        cout << "Graph for year " << year << " generated: " << filename << endl;
    }

    return 0;
}
