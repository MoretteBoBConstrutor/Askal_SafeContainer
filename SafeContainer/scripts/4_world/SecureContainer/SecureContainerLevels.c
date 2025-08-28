// Classe auxiliar para a estrutura do JSON (igual ao ASKNomadesListData)
class SecureContainerLevelsData
{
    ref array<string> level2;
    ref array<string> level3;
    ref array<string> level4;

    void SecureContainerLevelsData()
    {
        level2 = new array<string>;
        level3 = new array<string>;
        level4 = new array<string>;
    }
}

/**
 * @class SecureContainerLevels
 * @brief Gerencia os níveis de container premium dos jogadores.
 * 
 * Carrega uma lista de SteamID64 por nível de um arquivo JSON.
 * Igual ao ASKNomadesSystem mas para containers premium.
 */
class SecureContainerLevels
{
    private static const string FOLDER_PATH = "$profile:SafeContainer/";
    private static const string FILE_NAME = "SafeContainerLevels.json";
    
    // Armazena os dados dos níveis em memória
    private static ref SecureContainerLevelsData m_LevelsData;
    private static ref SecureContainerConfig s_SecureConfig;

    /**
     * @brief Carrega o arquivo SecureContainerLevels.json do perfil do servidor.
     * Deve ser chamado uma vez quando o servidor inicia.
     */
    static void LoadConfig()
    {
        if (m_LevelsData) return;

        // INICIALIZAÇÃO AUTOMÁTICA DO SISTEMA INDEPENDENTE
        Print("[SecureContainerLevels] Inicializando sistema de containers independente...");
        
        // Carrega configuração do sistema
        s_SecureConfig = new SecureContainerConfig();
        string secureConfigPath = FOLDER_PATH + "secure_container_config.json";
        s_SecureConfig.Load(secureConfigPath);
        Print("[SecureContainerLevels] Configuração do sistema carregada: " + secureConfigPath);
        
        // Inicializa o manager individual
        SecureContainerIndividualManager.GetInstance();
        Print("[SecureContainerLevels] Sistema de arquivos individuais inicializado");

        m_LevelsData = new SecureContainerLevelsData();
        
        // Garante que o diretório exista
        if (!FileExist(FOLDER_PATH))
        {
            Print("[SecureContainerLevels] Diretório não encontrado. Criando: " + FOLDER_PATH);
            MakeDirectory(FOLDER_PATH);
        }

        string filePath = FOLDER_PATH + FILE_NAME;
        
        if (FileExist(filePath))
        {
            Print("[SecureContainerLevels] Encontrado " + FILE_NAME + ". Carregando...");
            
            SecureContainerLevelsData data = new SecureContainerLevelsData();
            JsonFileLoader<SecureContainerLevelsData>.JsonLoadFile(filePath, data);

            if (data)
            {
                m_LevelsData = data;
                int totalPlayers = 0;
                if (m_LevelsData.level2) totalPlayers += m_LevelsData.level2.Count();
                if (m_LevelsData.level3) totalPlayers += m_LevelsData.level3.Count();  
                if (m_LevelsData.level4) totalPlayers += m_LevelsData.level4.Count();
                
                Print("[SecureContainerLevels] " + totalPlayers + " jogadores premium carregados com sucesso.");
            }
            else
            {
                //Print("[SecureContainerLevels] Falha ao carregar ou desserializar " + FILE_NAME);
            }
        }
        else
        {
            Print("[SecureContainerLevels] Arquivo " + FILE_NAME + " não encontrado. Criando um novo com exemplos...");
            
            // Cria estrutura exemplo com SteamIDs de exemplo
            SecureContainerLevelsData exampleData = new SecureContainerLevelsData();
            
            // Level 2 - Premium (30 slots)
            exampleData.level2.Insert("76561198181942294"); // <-- SUBSTITUA PELOS STEAMID64 REAIS
            exampleData.level2.Insert("76561198999999999");
            
            // Level 3 - VIP (36 slots)  
            exampleData.level3.Insert("76561198888888888");
            exampleData.level3.Insert("76561198777777777");
            
            // Level 4 - Ultra (48 slots)
            exampleData.level4.Insert("76561198666666666");
            exampleData.level4.Insert("76561198555555555");

            // Salva o novo arquivo JSON
            JsonFileLoader<SecureContainerLevelsData>.JsonSaveFile(filePath, exampleData);

            // Carrega os dados recém-criados para a memória
            m_LevelsData = exampleData;
            Print("[SecureContainerLevels] Arquivo exemplo criado com 6 jogadores de exemplo.");
        }
    }

    /**
     * @brief Verifica o nível de container de um jogador baseado no seu SteamID64.
     * @param steamID O SteamID64 do jogador a ser verificado.
     * @return 1=Padrão, 2=Premium, 3=VIP, 4=Ultra
     */
    static int GetPlayerContainerLevel(string steamID)
    {
        if (!m_LevelsData) LoadConfig();

        // Verifica em qual nível o jogador está - SIMPLES!
        if (m_LevelsData.level4 && m_LevelsData.level4.Find(steamID) != -1) return 4; // Ultra - 48 slots
        if (m_LevelsData.level3 && m_LevelsData.level3.Find(steamID) != -1) return 3; // VIP - 36 slots  
        if (m_LevelsData.level2 && m_LevelsData.level2.Find(steamID) != -1) return 2; // Premium - 30 slots

        return 1; // Padrão - 24 slots
    }

    /**
     * @brief Obtém a classe do container baseada no nível do jogador.
     * @param steamID O SteamID64 do jogador.
     * @return Nome da classe do container apropriado.
     */
    static string GetPlayerContainerClassName(string steamID)
    {
        int level = GetPlayerContainerLevel(steamID);
        
        if (level == 2) return "Askal_SecureContainer_Lv2";    // Premium - 30 slots
        if (level == 3) return "Askal_SecureContainer_Lv3";    // VIP - 36 slots  
        if (level == 4) return "Askal_SecureContainer_Lv4";    // Ultra - 48 slots
        
        return "Askal_SecureContainer"; // Padrão - 24 slots
    }

    /**
     * @brief Força recarregamento da configuração (para admins).
     */
    static void ReloadConfig()
    {
        m_LevelsData = null;
        LoadConfig();
        Print("[SecureContainerLevels] Configuração recarregada!");
    }

    /**
     * @brief Para debug - mostra o nível de um jogador.
     */
    static void CheckPlayerLevel(string steamID)
    {
        int level = GetPlayerContainerLevel(steamID);
        string className = GetPlayerContainerClassName(steamID);
        Print("[SecureContainerLevels] Player " + steamID + " = Nível " + level + " (" + className + ")");
    }

    /**
     * @brief Acesso ao SecureContainerConfig.
     */
    static SecureContainerConfig GetSecureContainerConfig()
    {
        if (!s_SecureConfig) LoadConfig();
        return s_SecureConfig;
    }
}
