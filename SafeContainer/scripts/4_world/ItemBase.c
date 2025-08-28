modded class ItemBase
{
    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        // Chama a verificação padrão primeiro
        if (!super.CanReceiveItemIntoCargo(item))
            return false;
        
        // ✅ VERIFICAR SE FAZ PARTE DA HIERARQUIA DE UM SECURECONTAINER
        EntityAI rootContainer = GetRootSecureContainer();
        Askal_SecureContainer secureContainer = Askal_SecureContainer.Cast(rootContainer);
        
        if (secureContainer)
        {
            // Se está na hierarquia de um SecureContainer, usar a verificação recursiva do SecureContainer
            if (!secureContainer.IsItemAllowedRecursive(item))
            {
                //Print("[ItemBase] Item REJEITADO em container da hierarquia do SecureContainer: " + item.GetType());
                return false;
            }
        }
        
        return true;
    }
    
    // ✅ FUNÇÃO AUXILIAR: Encontra o SecureContainer na hierarquia
    private EntityAI GetRootSecureContainer()
    {
        EntityAI current = this;
        
        // Sobe na hierarquia até encontrar SecureContainer ou chegar no topo
        while (current)
        {
            // Se encontrou um SecureContainer, retorna
            if (current.IsKindOf("Askal_SecureContainer"))
            {
                return current;
            }
            
            // Sobe um nível na hierarquia
            current = current.GetHierarchyParent();
        }
        
        return null; // Não está na hierarquia de um SecureContainer
    }
}