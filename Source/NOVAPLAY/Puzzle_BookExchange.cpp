#include "Puzzle_BookExchange.h"
#include "NOVAPLAYInventoryComponent.h"
#include "GameFramework/Pawn.h"

bool APuzzle_BookExchange::TryExchange(APawn* InteractingPawn)
{
	if (bIsSolved || !InteractingPawn)
	{
		return false;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// Primero VERIFICA que tenga todos los libros, sin sacar nada todavia --
	// si sacaramos a medida que chequeamos, un canje fallido a mitad de camino
	// dejaria al jugador con menos libros y sin la llave
	for (const FName& BookID : RequiredBookIDs)
	{
		if (!Inventory->HasItem(BookID))
		{
			return false; // le falta al menos uno, no se toca el inventario
		}
	}

	// Recien aca, con la certeza de que los tiene todos, se sacan y se entrega la llave
	for (const FName& BookID : RequiredBookIDs)
	{
		Inventory->RemoveItem(BookID, 1);
	}

	Inventory->AddItem(KeyItemID, KeyItemName, KeyIcon, KeyDescription, 1);

	OnSolved();
	return true;
}
