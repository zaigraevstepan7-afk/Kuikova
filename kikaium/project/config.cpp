#include "config.h"

namespace configs
{
    bool Input(const char *text, char *buf, bool *isOskOpened)
    {
        const auto w = ImGui::GetContentRegionAvail().x / 2.0f;
        const auto o = ImGui::GetCursorPosX();

        ImGui::TextUnformatted(text);
        ImGui::SameLine(o + w);

        const char *buttonText = buf[0] == 0 ? "Enter text" : buf;

        if (ImGui::Button(buttonText, {w, 0.0f}))
        {
            *isOskOpened = !*isOskOpened;
        }

        if (*isOskOpened)
        {
            if (ImGui::Begin(text, isOskOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (!ImGui::IsWindowFocused())
                {
                    *isOskOpened = false;
                }

                const float keySize = 50.0f;

                if (ImGui::Button("1", {keySize, keySize}))
                {
                    strcat(buf, "1");
                }
                ImGui::SameLine();
                if (ImGui::Button("2", {keySize, keySize}))
                {
                    strcat(buf, "2");
                }
                ImGui::SameLine();
                if (ImGui::Button("3", {keySize, keySize}))
                {
                    strcat(buf, "3");
                }
                ImGui::SameLine();
                if (ImGui::Button("4", {keySize, keySize}))
                {
                    strcat(buf, "4");
                }
                ImGui::SameLine();
                if (ImGui::Button("5", {keySize, keySize}))
                {
                    strcat(buf, "5");
                }
                ImGui::SameLine();
                if (ImGui::Button("6", {keySize, keySize}))
                {
                    strcat(buf, "6");
                }
                ImGui::SameLine();
                if (ImGui::Button("7", {keySize, keySize}))
                {
                    strcat(buf, "7");
                }
                ImGui::SameLine();
                if (ImGui::Button("8", {keySize, keySize}))
                {
                    strcat(buf, "8");
                }
                ImGui::SameLine();
                if (ImGui::Button("9", {keySize, keySize}))
                {
                    strcat(buf, "9");
                }
                ImGui::SameLine();
                if (ImGui::Button("0", {keySize, keySize}))
                {
                    strcat(buf, "0");
                }
                ImGui::SameLine();
                if (ImGui::Button("_", {keySize, keySize}))
                {
                    strcat(buf, "_");
                }

                if (ImGui::Button("Q", {keySize, keySize}))
                {
                    strcat(buf, "Q");
                }
                ImGui::SameLine();
                if (ImGui::Button("W", {keySize, keySize}))
                {
                    strcat(buf, "W");
                }
                ImGui::SameLine();
                if (ImGui::Button("E", {keySize, keySize}))
                {
                    strcat(buf, "E");
                }
                ImGui::SameLine();
                if (ImGui::Button("R", {keySize, keySize}))
                {
                    strcat(buf, "R");
                }
                ImGui::SameLine();
                if (ImGui::Button("T", {keySize, keySize}))
                {
                    strcat(buf, "T");
                }
                ImGui::SameLine();
                if (ImGui::Button("Y", {keySize, keySize}))
                {
                    strcat(buf, "Y");
                }
                ImGui::SameLine();
                if (ImGui::Button("U", {keySize, keySize}))
                {
                    strcat(buf, "U");
                }
                ImGui::SameLine();
                if (ImGui::Button("I", {keySize, keySize}))
                {
                    strcat(buf, "I");
                }
                ImGui::SameLine();
                if (ImGui::Button("O", {keySize, keySize}))
                {
                    strcat(buf, "O");
                }
                ImGui::SameLine();
                if (ImGui::Button("P", {keySize, keySize}))
                {
                    strcat(buf, "P");
                }
                ImGui::SameLine();
                if (ImGui::Button("<", {keySize, keySize}))
                {
                    size_t len = strlen(buf);
                    if (len > 0)
                        buf[len - 1] = '\0';
                }

                ImGui::Dummy({keySize, keySize});
                ImGui::SameLine();
                if (ImGui::Button("A", {keySize, keySize}))
                {
                    strcat(buf, "A");
                }
                ImGui::SameLine();
                if (ImGui::Button("S", {keySize, keySize}))
                {
                    strcat(buf, "S");
                }
                ImGui::SameLine();
                if (ImGui::Button("D", {keySize, keySize}))
                {
                    strcat(buf, "D");
                }
                ImGui::SameLine();
                if (ImGui::Button("F", {keySize, keySize}))
                {
                    strcat(buf, "F");
                }
                ImGui::SameLine();
                if (ImGui::Button("G", {keySize, keySize}))
                {
                    strcat(buf, "G");
                }
                ImGui::SameLine();
                if (ImGui::Button("H", {keySize, keySize}))
                {
                    strcat(buf, "H");
                }
                ImGui::SameLine();
                if (ImGui::Button("J", {keySize, keySize}))
                {
                    strcat(buf, "J");
                }
                ImGui::SameLine();
                if (ImGui::Button("K", {keySize, keySize}))
                {
                    strcat(buf, "K");
                }
                ImGui::SameLine();
                if (ImGui::Button("L", {keySize, keySize}))
                {
                    strcat(buf, "L");
                }

                if (ImGui::Button("Z", {keySize, keySize}))
                {
                    strcat(buf, "Z");
                }
                ImGui::SameLine();
                if (ImGui::Button("X", {keySize, keySize}))
                {
                    strcat(buf, "X");
                }
                ImGui::SameLine();
                if (ImGui::Button("C", {keySize, keySize}))
                {
                    strcat(buf, "C");
                }
                ImGui::SameLine();
                if (ImGui::Button("V", {keySize, keySize}))
                {
                    strcat(buf, "V");
                }
                ImGui::SameLine();
                if (ImGui::Button("B", {keySize, keySize}))
                {
                    strcat(buf, "B");
                }
                ImGui::SameLine();
                if (ImGui::Button("N", {keySize, keySize}))
                {
                    strcat(buf, "N");
                }
                ImGui::SameLine();
                if (ImGui::Button("M", {keySize, keySize}))
                {
                    strcat(buf, "M");
                }
                ImGui::SameLine();
                if (ImGui::Button(",", {keySize, keySize}))
                {
                    strcat(buf, ",");
                }
                ImGui::SameLine();
                if (ImGui::Button(".", {keySize, keySize}))
                {
                    strcat(buf, ".");
                }
                ImGui::SameLine();
                if (ImGui::Button("/", {keySize, keySize}))
                {
                    strcat(buf, "/");
                }
                ImGui::SameLine();
                if (ImGui::Button("\"", {keySize, keySize}))
                {
                    strcat(buf, "\"");
                }

                if (ImGui::Button("Space", {ImGui::GetContentRegionAvail().x, 0.0f}))
                {
                    strcat(buf, " ");
                }

                ImGui::End();
            }
            return !*isOskOpened;
        }

        return false;
    }

    const char *configsFolder = ("/storage/emulated/0/Documents");

    void cfgList(std::vector<std::string> &vector)
    {
        vector.clear();

        auto dir = opendir(configsFolder);
        if (dir)
        {
            dirent *ptr;
            while ((ptr = readdir(dir)))
            {
                if (ptr->d_type == DT_REG)
                {
                    const char *name = ptr->d_name;
                    if (strstr(name, (".CFG")) || strstr(name, (".cfg")))
                    {
                        vector.emplace_back(name);
                    }
                }
            }
            closedir(dir);
        }
    }

    void cfgRead(const char *filename)
    {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", configsFolder, filename);

        auto fp = fopen(path, "rb");
        if (fp)
        {
            fread(&g, sizeof(g), 1, fp);
            fclose(fp);
        }
    }

    void cfgWrite(const char *filename = nullptr)
    {
        char actualFilename[128];

        if (filename == nullptr || strlen(filename) == 0)
        {
            std::vector<std::string> existingConfigs;
            cfgList(existingConfigs);

            int configNumber = 1;
            char testName[128];

            while (configNumber < 1000)
            {
                snprintf(testName, sizeof(testName), "mel_%d.cfg", configNumber);
                bool exists = false;

                for (const auto &config : existingConfigs)
                {
                    if (config == testName)
                    {
                        exists = true;
                        break;
                    }
                }

                if (!exists)
                    break;
                configNumber++;
            }

            if (configNumber >= 1000)
            {
                snprintf(testName, sizeof(testName), "mel_%lld.cfg",
                         std::chrono::system_clock::now().time_since_epoch().count());
            }

            strcpy(actualFilename, testName);
        }
        else
        {
            strncpy(actualFilename, filename, sizeof(actualFilename) - 1);
            actualFilename[sizeof(actualFilename) - 1] = '\0';

            if (!strstr(actualFilename, ".cfg") && !strstr(actualFilename, ".CFG"))
            {
                strncat(actualFilename, ".cfg", sizeof(actualFilename) - strlen(actualFilename) - 1);
            }
        }

        char path[256];
        snprintf(path, sizeof(path), "%s/%s", configsFolder, actualFilename);

        auto fp = fopen(path, "wb");
        if (fp)
        {
            fwrite(&g, sizeof(g), 1, fp);
            fclose(fp);
        }
    }
}
