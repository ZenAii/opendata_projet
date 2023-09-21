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

void histogramme_CO2(const string &filename, const string &title, const vector<string> &years, const vector<double> &rates)
{
    const int image_width = 1000; // Largeur de l'image
    const int image_height = 800; // Hauteur de l'image

    int barWidth = 20;   // Largeur de chaque barre
    int barSpacing = 40; // Espace entre les barres

    // Calculer la largeur totale des barres et l'espacement total entre elles
    int totalBarWidth = (barWidth + barSpacing) * years.size() - barSpacing;
    int startX = 100;                // Position de départ pour les barres
    int startY = image_height - 100; // Position de départ pour les barres

    gdImagePtr im = gdImageCreateTrueColor(image_width, image_height);
    int white = gdImageColorAllocate(im, 255, 255, 255);
    int black = gdImageColorAllocate(im, 0, 0, 0);
    int textColor = gdImageColorAllocate(im, 0, 0, 0);

    gdImageFill(im, 0, 0, white);

    int titleX = (image_width - gdFontGetLarge()->w * title.length()) / 2; // Centrer le titre horizontalement

    gdImageString(im, gdFontGetLarge(), titleX, 50, (unsigned char *)title.c_str(), textColor); // Position centrale en haut
    gdImageStringUp(im, gdFontGetSmall(), 20, startY - 350, (unsigned char *)"CO2 rate (g/kWh)", textColor);

    // Dessiner l'axe des X jusqu'au bord droit de l'image
    gdImageLine(im, startX, startY, image_width - 100, startY, black); // Ajustez la position finale du trait des X

    // Dessiner l'axe des Y
    gdImageLine(im, startX, startY, startX, startY - 500, black); // Ajustez la hauteur de l'axe Y

    // Ajouter des étiquettes numériques sur l'axe Y avec des chiffres ronds
    int numLabels = 5;       // Nombre d'étiquettes numériques
    int labelIncrement = 20; // Incrémentation des étiquettes
    int startYValue = 0;     // Valeur de départ

    for (int i = 0; i <= numLabels; i++)
    {
        int labelValue = startYValue + i * labelIncrement;
        int labelY = startY - static_cast<int>((labelValue - 0) / 100.0 * 500); // Ajustez la hauteur de l'axe Y

        // Afficher la valeur de l'étiquette
        stringstream labelStream;
        labelStream << labelValue << "%";

        gdImageString(im, gdFontGetSmall(), startX - 40, labelY, (unsigned char *)labelStream.str().c_str(), textColor);
    }

    for (int i = 0; i < years.size(); i++)
    {
        // Normalisez la valeur CO2 pour qu'elle se situe entre 0% et 100%
        double normalizedRate = (rates[i] <= 100.0) ? rates[i] : 100.0;

        // Ajustez la hauteur des barres pour le zoom
        int barHeight = static_cast<int>((normalizedRate / 100.0) * 500); // Ajustez l'échelle de la hauteur

        // Ajustez la position des barres pour le zoom
        int x1 = startX + (barWidth + barSpacing) * i;
        int y1 = startY - barHeight;
        int x2 = x1 + barWidth;
        int y2 = startY;

        // Utiliser différentes couleurs pour chaque barre
        int color = gdImageColorAllocate(im, 0, 0, 255); // Couleur bleue pour les barres du total des années
        gdImageFilledRectangle(im, x1, y1, x2, y2, color);

        // Calculer la position horizontale centrée pour le titre de chaque barre
        int titleX = x1 + barWidth / 2 - gdFontGetSmall()->w / 2;
        gdImageString(im, gdFontGetSmall(), titleX, startY + 20, (unsigned char *)years[i].c_str(), textColor);
    }

    // Enregistrement de l'image dans le dossier "graphique"
    FILE *out = fopen(filename.c_str(), "wb");
    gdImagePng(im, out);
    fclose(out);

    gdImageDestroy(im);
}

int main()
{
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

    // Obtenir la liste des sous-secteurs uniques
    vector<string> uniqueSubsectors;
    for (const DataPoint &dataPoint : dataPoints)
    {
        if (std::find(uniqueSubsectors.begin(), uniqueSubsectors.end(), dataPoint.subsector) == uniqueSubsectors.end())
        {
            uniqueSubsectors.push_back(dataPoint.subsector);
        }
    }

    // Générer un graphique pour chaque sous-secteur unique
    for (const string &subsector : uniqueSubsectors)
    {
        vector<double> ratesForSubsector;
        vector<string> yearsForSubsector;

        // Filtrer les données pour le sous-secteur spécifique
        for (const DataPoint &dataPoint : dataPoints)
        {
            if (dataPoint.subsector == subsector)
            {
                ratesForSubsector.push_back(dataPoint.rate);
                yearsForSubsector.push_back(dataPoint.year);
            }
        }

        // Spécifier le chemin complet du fichier de sortie dans le dossier "graphique"
        string filename = "graphique/emissionCO2_" + subsector + ".png";

        // Générer le graphique pour ce sous-secteur
        histogramme_CO2(filename, subsector, yearsForSubsector, ratesForSubsector);
        cout << "Graph for subsector " << subsector << " generated: " << filename << endl;
    }

    // Générer un graphique pour chaque année avec la somme totale des émissions de CO2
    vector<string> uniqueYears;
    for (const DataPoint &dataPoint : dataPoints)
    {
        if (std::find(uniqueYears.begin(), uniqueYears.end(), dataPoint.year) == uniqueYears.end())
        {
            uniqueYears.push_back(dataPoint.year);
        }
    }

    for (const string &year : uniqueYears)
    {
        double totalCO2ForYear = 0.0;

        // Filtrer les données pour l'année spécifique
        for (const DataPoint &dataPoint : dataPoints)
        {
            if (dataPoint.year == year)
            {
                totalCO2ForYear += dataPoint.rate;
            }
        }

        // Spécifier le chemin complet du fichier de sortie dans le dossier "graphique"
        string filename = "graphique/emissionCO2_" + year + ".png";

        // Générer le graphique pour cette année avec la somme totale des émissions de CO2
        histogramme_CO2(filename, year, vector<string>{year}, vector<double>{totalCO2ForYear});
        cout << "Graph for year " << year << " generated: " << filename << endl;
    }

    return 0;
}
