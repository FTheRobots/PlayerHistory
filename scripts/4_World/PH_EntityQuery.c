class PH_EntityQuery
{
    static void GetEntitiesInWorldBox(out array<EntityAI> entities, int queryFlags = QueryFlags.DYNAMIC)
    {
        if (!entities)
            entities = new array<EntityAI>();
        else
            entities.Clear();

        float worldSize = GetGame().GetWorld().GetWorldSize();
        vector min = Vector(0, -1200, 0);
        vector max = Vector(worldSize, 1200, worldSize);
        DayZPlayerUtils.SceneGetEntitiesInBox(min, max, entities, queryFlags);
    }

    static void GetEntitiesInBox(vector center, float horizontalRadius, float verticalHalfHeight, out array<EntityAI> entities, int queryFlags = QueryFlags.DYNAMIC)
    {
        if (!entities)
            entities = new array<EntityAI>();
        else
            entities.Clear();

        if (horizontalRadius <= 0)
            return;

        if (verticalHalfHeight <= 0)
            verticalHalfHeight = 1.5;

        vector min = Vector(center[0] - horizontalRadius, center[1] - verticalHalfHeight, center[2] - horizontalRadius);
        vector max = Vector(center[0] + horizontalRadius, center[1] + verticalHalfHeight, center[2] + horizontalRadius);
        DayZPlayerUtils.SceneGetEntitiesInBox(min, max, entities, queryFlags);
    }
};
