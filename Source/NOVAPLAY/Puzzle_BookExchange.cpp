#include "Puzzle_BookExchange.h"
#include "GameFramework/Pawn.h"
#include "NOVAPLAY.h"
#include "NOVAPLAYInventoryComponent.h"

bool APuzzle_BookExchange::TryExchange(APawn* InteractingPawn)
{
	if (bIsSolved || !InteractingPawn)
	{
		return false;
	}

	// Sin ID de llave el AddItem de mas abajo fallaria DESPUES de haberle sacado los libros
	// al jugador (perderia todo y no recibiria nada): se corta antes de tocar el inventario.
	if (KeyItemID.IsNone())
	{
		UE_LOG(LogNOVAPLAY, Error, TEXT("%s: KeyItemID no esta configurado, no se puede hacer el canje."), *GetName());
		return false;
	}

	UNOVAPLAYInventoryComponent* Inventory = InteractingPawn->FindComponentByClass<UNOVAPLAYInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	// Cuantas unidades de cada libro hacen falta (por si un ID esta repetido en la lista)
	TMap<FName, int32> RequiredCounts;
	for (const FName& BookID : RequiredBookIDs)
	{
		RequiredCounts.FindOrAdd(BookID)++;
	}

	// Primero VERIFICA que tenga todos los libros, sin sacar nada todavia --
	// si sacaramos a medida que chequeamos, un canje fallido a mitad de camino
	// dejaria al jugador con menos libros y sin la llave
	for (const TPair<FName, int32>& Required : RequiredCounts)
	{
		if (Inventory->GetItemQuantity(Required.Key) < Required.Value)
		{
			return false; // le falta al menos uno, no se toca el inventario
		}
	}

	// Recien aca, con la certeza de que los tiene todos, se sacan y se entrega la llave
	for (const TPair<FName, int32>& Required : RequiredCounts)
	{
		Inventory->RemoveItem(Required.Key, Required.Value);
	}

	Inventory->AddItem(KeyItemID, KeyItemName, KeyIcon, KeyDescription, 1);

	OnSolved();
	return true;
}
