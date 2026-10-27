// Preserve the recoverable original if either replacement or rollback fails.
// Cleanup failure after a successful install must never roll back the new file.
export async function replaceWithBackup(io, temporary, destination, backup) {
  const backedUp = await io.exists(destination)
  if (backedUp) await io.rename(destination, backup)
  try {
    await io.rename(temporary, destination)
  } catch (error) {
    if (backedUp) {
      try {
        if (await io.exists(destination)) await io.remove(destination)
        await io.rename(backup, destination)
      } catch (rollback) {
        throw new Error(`Replacement failed: ${error.message}. Restore also failed: ${rollback.message}. Original file retained at ${backup}`, {cause:error})
      }
    }
    throw error
  }
  if (backedUp) {
    try { await io.remove(backup) }
    catch (error) { throw new Error(`New file saved at ${destination}, but backup cleanup failed. Original retained at ${backup}: ${error.message}`, {cause:error}) }
  }
}
