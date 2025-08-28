modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        // SISTEMA INDEPENDENTE: Inicializa sistema de Secure Containers
        SecureContainerLevels.LoadConfig();
        Print("[MissionServer] Sistema de Secure Containers independente inicializado");
    }

    override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
    {
        // 1. Chama a lógica original do DayZ. Isso irá criar o personagem
        // e popular a variável de membro 'm_player' na classe base.
        super.OnClientNewEvent(identity, pos, ctx);

        // 2. Agora que 'm_player' está definido, podemos usá-lo para agendar nossa lógica.
        // Usar CallLater ainda é a abordagem mais segura para evitar condições de corrida.
        if (m_player)
        {
            // SISTEMA SECURE CONTAINER - Dá o container e restaura os itens salvos
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.SetupSecureContainer, 1700, false, m_player);
        }

        // 3. Retorna a variável de membro 'm_player', seguindo o padrão da função original.
        return m_player;
    }

    // SISTEMA SECURE CONTAINER
    void SetupSecureContainer(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
            
        string playerUID = player.GetIdentity().GetPlainId();
        //Print("[MissionServer] Configurando secure container para: " + playerUID);
        
        // 1. PRIMEIRO - Garantir que o jogador tem o container anexado
        player.GivePlayerSecureContainer();
        
        // 2. SEGUNDO - Tentar restaurar os itens salvos de mortes anteriores
        SecureContainerIndividualManager.GetInstance().RestoreSecureContainerContents(player);
        
        //Print("[MissionServer] Secure container configurado para: " + playerUID);
    }
}